#include "scribl.h"
#include "reverse_sem.h"
#include "logging.h"

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define CHECK_ELEMENT(e) ((e != NULL) && (e->data != NULL))

static gpointer serialize_ht(gpointer data);
static gpointer event_loop_worker(gpointer data);

static GMutex *counter_list_access;
static GSList *counter_list;

static GReverseSemaphore *scribl_exit_semaphore = NULL;

static GMutex *event_worker_access;
static GCond *event_worker_cond;

static GThread *event_worker_thd;

void scribl_init(double wakeup_interval)
{
	double *wake_interval_ptr;

	if (!g_thread_supported())
		g_thread_init (NULL);

	g_log_set_default_handler(scribl_logfunc, NULL);

	scribl_exit_semaphore = g_reverse_semaphore_create();
	g_assert(scribl_exit_semaphore != NULL);

	counter_list_access = g_mutex_new();
	counter_list = g_slist_alloc();

	event_worker_cond = g_cond_new();
	event_worker_access = g_mutex_new();

	/* Spawn a new event-loop thread. */
	wake_interval_ptr = g_slice_copy(sizeof(double), &wakeup_interval);
	event_worker_thd = g_thread_create(event_loop_worker, wake_interval_ptr, TRUE, NULL);
}

void scribl_exit()
{
	GSList *element;

	g_reverse_semaphore_destroy(scribl_exit_semaphore);
	scribl_exit_semaphore = NULL;

	/* Signal the event worker to end, and join it */
	g_cond_signal(event_worker_cond);
	g_thread_join(event_worker_thd);

	/* Destroy any outstanding counters. */
	g_mutex_lock(counter_list_access);
	for (element = counter_list; CHECK_ELEMENT(element); element = counter_list->next)
		scribl_free_counter(element->data);

	g_mutex_unlock(counter_list_access);
	g_mutex_free(counter_list_access);
	g_slist_free(counter_list);
}

static void ht_val_free(gpointer data)
{
	g_slice_free(double, data);
}

struct scribl_counter* scribl_new_counter(const char *name)
{
	struct scribl_counter *counter;
	
	counter = g_slice_alloc(sizeof(struct scribl_counter));
	counter->name = g_strdup(name);
	counter->ht = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, ht_val_free);
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
	gchar *key_cpy;
	gpointer *val;

	g_assert(counter != NULL);
	g_assert(key != NULL);

	g_mutex_lock(counter->lock);

	val = g_hash_table_lookup(counter->ht, key);
	if (val == NULL) {
		key_cpy = g_strdup(key);
		val = g_slice_alloc0(sizeof(double));
		g_hash_table_insert(counter->ht, key_cpy, val);
	}
	return val;
}

/* This increments a key in for ht[major][minor] */
void scribl_incr_counter(struct scribl_counter *counter, char *key, double incr)
{
	double *current_val;

	current_val = get_val_ptr(counter, key);
	*current_val += incr;
	g_mutex_unlock(counter->lock);
}

void scribl_free_counter(struct scribl_counter *counter)
{
	/* Remove from the global list first */
	g_mutex_lock(counter_list_access);
	counter_list = g_slist_remove(counter_list, counter);
	g_mutex_unlock(counter_list_access);

	g_mutex_lock(counter->lock);
	if (counter->name)
		g_free(counter->name);
	g_hash_table_destroy(counter->ht);
	/* FIXME: is this correct? */
	g_mutex_unlock(counter->lock);
	g_mutex_free(counter->lock);

	g_slice_free(struct scribl_counter, counter);
}

double scribl_lookup_counter(struct scribl_counter *counter, char *key)
{
	double *val;

	g_mutex_lock(counter->lock);
	val = g_hash_table_lookup(counter->ht, key);
	g_mutex_unlock(counter->lock);

	return (val == NULL) ? 0 : *val;
}

static gpointer serialize_ht(gpointer data)
{
	GHashTable *tbl;
	GHashTableIter iter;
	GTimeVal tv;
	gpointer key, value;
	gpointer **arr = data;
	size_t fname_len;
	int err;

	FILE *log_file;

	gchar *data_name;
	char *fname;

	data_name = (char *) arr[0];
	tbl = (GHashTable *) arr[1];
	g_slice_free1(2 * sizeof(gpointer), data);

	fname_len = 32 + strlen(data_name);
	fname = g_slice_alloc(fname_len);

	g_get_current_time(&tv);

	if (data_name != NULL)
		err = sprintf(fname, "scribl-%s-%ld.log", data_name, tv.tv_sec);
	else
		err = sprintf(fname, "scribl-%ld.log", tv.tv_sec);

	if (err < 0)
		perror("sprintf");

	log_file = fopen(fname, "w");
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "Serializing GHashTable at %p to %s", data, fname);
	g_slice_free1(fname_len, fname);
	g_assert(log_file != NULL);

	g_hash_table_iter_init(&iter, tbl);

	while (g_hash_table_iter_next(&iter, &key, &value)) {
		if (fprintf(log_file, "%s %f\n", (char *) key, *((double *) value)) < 0) {
			perror("fprintf");
		}
	}

	if (fclose(log_file) != 0)
		perror("fclose");
	g_hash_table_destroy(tbl);
	g_free(data_name);

	return NULL;
}

/* This represents a thread that occasionally serializes data structures. */
static gpointer event_loop_worker(gpointer data)
{
	glong sleep_duration, sd;
	glong time_elapsed;
	GHashTable *new_ht, *old_ht;
	GSList *element;

	/* Serialization things */
	gpointer *serialize_data;
	gchar *ser_name;

	struct scribl_counter *counter;

	/* The start and end time, used for timing the loop. */
	GTimeVal ts, te;

	sleep_duration = (glong) (*((double *) data)) * G_USEC_PER_SEC;
	g_slice_free(double, data);

	/* Wait for the sleep duration before entering the loop, to avoid racing
	 * with code that creates counters immediately after invoking
	 * scribl_init. */
	/* FIXME */

	while (TRUE) {
		g_reverse_semaphore_up(scribl_exit_semaphore);
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

		for (element = counter_list; CHECK_ELEMENT(element); element = counter_list->next) {
			counter = (struct scribl_counter *) element->data;

			new_ht = g_hash_table_new(g_str_hash, g_str_equal);

			/* Swap out the hash table. */
			g_mutex_lock(counter->lock);
			old_ht = counter->ht;
			counter->ht = new_ht;
			ser_name = g_strdup(counter->name);
			g_mutex_unlock(counter->lock);

			/* Spawn a serialization thread. */
			serialize_data = g_slice_alloc(2 * sizeof(gpointer));
			serialize_data[0] = ser_name;
			serialize_data[1] = old_ht;
			g_thread_create(serialize_ht, serialize_data, FALSE, NULL);
		}

		g_mutex_unlock(counter_list_access);

		if (scribl_exit_semaphore == NULL) {
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Breaking early");
			break;
		}

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
		g_assert(time_elapsed > 0);
		sd = sleep_duration - time_elapsed;
		g_assert(sd > 0);

		/* Sleep, wake up when it's time to flush data again. The exit semaphore
		 * is released during sleep. */
		g_reverse_semaphore_down(scribl_exit_semaphore);

		g_get_current_time(&te);
		g_time_val_add(&te, sd);

		g_mutex_lock(event_worker_access);
		if (g_cond_timed_wait(event_worker_cond, event_worker_access, &te) == TRUE) {
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Event worker condition was signaled");
			if (scribl_exit_semaphore == NULL) {
				g_mutex_unlock(event_worker_access);
				break;
			}
		}
		g_mutex_unlock(event_worker_access);
		g_assert(scribl_exit_semaphore != NULL);
	}

	return NULL;
}
