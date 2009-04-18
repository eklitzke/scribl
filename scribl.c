#include "scribl.h"
#include "reverse_sem.h"

#include <glib.h>
#include <stdio.h>

static gpointer serialize_ht(gpointer data);
static gpointer event_worker(gpointer data);

static GMutex *counter_list_access;
static GSList *counter_list;

GReverseSemaphore *scribl_exit_semaphore = NULL;

/**
 * Initialize scribl.
 * This method must be called before calling any other methods exposed by
 * scribl. It must only be called once.
 */
void scribl_init()
{
	if (!g_thread_supported())
		g_thread_init (NULL);
	scribl_exit_semaphore = g_reverse_semaphore_create();

	counter_list_access = g_mutex_new();
	counter_list = g_slist_alloc();

	/* Spawn a new event-loop thread. */
	g_thread_create(event_worker, NULL, 0, NULL);
}

void scribl_exit()
{
	g_reverse_semaphore_destroy(scribl_exit_semaphore);
	scribl_exit_semaphore = NULL;
}

static void val_guint_free(gpointer data)
{
	g_slice_free(guint, data);
}

struct scribl_counter* scribl_new_counter()
{
	struct scribl_counter *counter;
	
	counter = g_slice_alloc(sizeof(struct scribl_counter));
	counter->ht = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, val_guint_free);
	counter->lock = g_mutex_new();

	/* Add the list to the global counter list */
	g_mutex_lock(counter_list_access);
	counter_list = g_slist_prepend(counter_list, counter);
	g_mutex_unlock(counter_list_access);

	return counter;
}

/* This locks the mutex on counter, but does *NOT* unlock it. */
static gpointer get_val_ptr(struct scribl_counter *counter, char *key)
{
	GHashTable *sub;
	gchar *key_cpy;
	gpointer *val;

	g_assert(counter != NULL);
	g_assert(key != NULL);

	g_mutex_lock(counter->lock);

	val = g_hash_table_lookup(counter->ht, key);
	if (val == NULL) {
		key_cpy = g_strdup(key);
		val = g_slice_alloc0(sizeof(guint));
		g_hash_table_insert(counter->ht, key_cpy, val);
	}
	return val;
}

/* This increments a key in for ht[major][minor] */
void scribl_incr_counter(struct scribl_counter *counter, char *key)
{
	guint *val;

	val = get_val_ptr(counter, key);
	(*val)++;
	g_mutex_unlock(counter->lock);
}

void scribl_free_counter(struct scribl_counter *counter)
{
	GHashTableIter iter_i, iter_j;
	gpointer key_i, value_i, key_j, value_j;

	/* Remove from the global list first */
	g_mutex_lock(counter_list_access);
	counter_list = g_slist_remove(counter_list, counter);
	g_mutex_unlock(counter_list_access);

	g_mutex_lock(counter->lock);
	g_hash_table_destroy(counter->ht);
	/* FIXME: is this correct? */
	g_mutex_unlock(counter->lock);
	g_mutex_free(counter->lock);

	g_slice_free(struct scribl_counter, counter);
}

guint scribl_lookup_counter(struct scribl_counter *counter, char *key)
{
	GHashTable *sub;
	guint *val;

	g_mutex_lock(counter->lock);
	val = g_hash_table_lookup(counter->ht, key);
	g_mutex_unlock(counter->lock);

	return (val == NULL) ? 0 : *val;
}

gpointer serialize_ht(gpointer data)
{
	GHashTableIter iter;
	gpointer key, value;

	g_hash_table_iter_init(&iter, (GHashTable *) data);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		printf("serializing %p: %s -> %d\n", data, (char *) key, *((guint *) value));
	}
	g_hash_table_destroy((GHashTable *) data);
	return NULL;
}

/* This represents a thread that occasionally serializes data structures. */
gpointer event_worker(gpointer data)
{
	gulong sleep_interval;
	gulong sleep_duration = 5 * G_USEC_PER_SEC;
	gulong time_elapsed;
	GHashTable *new_ht, *old_ht;
	GSList *element;
	struct scribl_counter *counter;

	/* The start and end time, used for timing the loop. */
	GTimeVal ts, te;

	while (1) {
		g_get_current_time(&ts);

		/* Lock the global counter list. No new counters may be created while
		 * this lock is held.
		 *
		 * TODO: In most circumstances creating new locks
		 * should be ok while iterating through the list (especially when new
		 * counters are created using prepend). And in fact generally this is
		 * true of removing counters. This could be optimized to use better
		 * locking. */
		g_mutex_lock(counter_list_access);

		for (element = counter_list; (element != NULL) && (element->data != NULL); element = counter_list->next) {
			counter = (struct scribl_counter *) element->data;

			new_ht = g_hash_table_new(g_str_hash, g_str_equal);

			/* Swap out the hash table. */
			g_mutex_lock(counter->lock);
			old_ht = counter->ht;
			counter->ht = new_ht;
			g_mutex_unlock(counter->lock);

			/* Spawn a serialization thread. */
			g_thread_create(serialize_ht, old_ht, FALSE, NULL);
		}

		g_mutex_unlock(counter_list_access);

		g_get_current_time(&te);

		/* Since the work done by this thread should be done quickly, it's
		 * likely that it will take much less than a second. */
		if (G_LIKELY(te.tv_sec == ts.tv_sec))
			time_elapsed = te.tv_usec - ts.tv_usec;
		else {
			time_elapsed = G_USEC_PER_SEC - ts.tv_usec;
			time_elapsed += te.tv_usec;
			time_elapsed += G_USEC_PER_SEC * (te.tv_sec - ts.tv_sec - 1);
		}

		/* Sleep, wake up when it's time to flush data again. */
		g_usleep(sleep_duration - time_elapsed);
	}
}
