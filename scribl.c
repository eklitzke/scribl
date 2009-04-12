#include "scribl.h"
#include <glib.h>

void scribl_init()
{
	if (!g_thread_supported())
		g_thread_init (NULL);
}

struct scribl_counter* scribl_new_counter()
{
	struct scribl_counter *counter;
	
	counter = g_slice_alloc(sizeof(struct scribl_counter));
	counter->ht = g_hash_table_new(g_str_hash, g_str_equal);
	counter->lock = g_mutex_new();
	return counter;
}

/* This locks the mutex on counter, but does *NOT* unlock it. */
static gpointer get_val_ptr(struct scribl_counter *counter, char *major, char *minor)
{
	GHashTable *sub;
	gchar *major_cpy, *minor_cpy;
	gpointer *val;

	g_mutex_lock(counter->lock);

	sub = g_hash_table_lookup(counter->ht, major);
	if (sub == NULL) {
		major_cpy = g_strdup(major);
		sub = g_hash_table_new(g_str_hash, g_str_equal);
		g_hash_table_insert(counter->ht, major_cpy, sub);
	}

	val = g_hash_table_lookup(sub, minor);
	if (val == NULL) {
		minor_cpy = g_strdup(minor);
		val = g_slice_alloc0(sizeof(guint));
		g_hash_table_insert(sub, minor_cpy, val);
	}
	return val;
}

/* This increments a key in for ht[major][minor] */
void scribl_incr_counter(struct scribl_counter *counter, char *major, char *minor)
{
	guint *val;

	val = get_val_ptr(counter, major, minor);
	(*val)++;
	g_mutex_unlock(counter->lock);
}

void scribl_free_counter(struct scribl_counter *counter)
{
	GHashTableIter iter_i, iter_j;
	gpointer key_i, value_i, key_j, value_j;

	g_mutex_lock(counter->lock);

	g_hash_table_iter_init(&iter_i, counter->ht);
	while (g_hash_table_iter_next(&iter_i, &key_i, &value_i)) {
		g_hash_table_iter_remove(&iter_i);
		g_free(key_i);

		g_hash_table_iter_init(&iter_j, value_i);
		while (g_hash_table_iter_next(&iter_j, &key_j, &value_j)) {
			g_hash_table_iter_remove(&iter_j);
			g_free(key_j);
			g_slice_free(guint, value_j);
		}
		g_hash_table_destroy(value_i);
	}

	/* FIXME: is this correct? */
	g_mutex_unlock(counter->lock);
	g_mutex_free(counter->lock);

	g_slice_free(struct scribl_counter, counter);
}

guint scribl_lookup_counter(struct scribl_counter *counter, char *major, char *minor)
{
	GHashTable *sub;
	guint *val;

	g_mutex_lock(counter->lock);

	sub = g_hash_table_lookup(counter->ht, major);
	if (sub == NULL)
		goto return_zero;

	val = g_hash_table_lookup(sub, minor);
	if (val == NULL)
		goto return_zero;

	g_mutex_unlock(counter->lock);
	return *val;

return_zero:
	g_mutex_unlock(counter->lock);
	return 0;
}
