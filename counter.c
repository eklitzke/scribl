/* This uses hash tables all over the place, which isn't optimal.
 */

#include <glib.h>

/* Create a new counter. This is really just a hash table. */
GHashTable* new_counter()
{
	GHashTable *ht;
	ht = g_hash_table_new(g_str_hash, g_str_equal);
	return ht;
}

/* This increments a key in for ht[major][minor] */
void incr_counter(GHashTable *ht, char *major, char *minor)
{
	GHashTable *sub;
	gchar *major_cpy, *minor_cpy;
	guint *val;

	sub = g_hash_table_lookup(ht, major);
	if (sub == NULL) {
		major_cpy = g_strdup(major);
		sub = g_hash_table_new(g_str_hash, g_str_equal);
		g_hash_table_insert(ht, major_cpy, sub);
	}

	val = g_hash_table_lookup(sub, minor);
	if (val == NULL) {
		minor_cpy = g_strdup(minor);
		val = g_slice_alloc0(sizeof(guint));
		g_hash_table_insert(sub, minor_cpy, val);
	}
	(*val)++;
}

void free_counter(GHashTable *ht)
{
	GHashTableIter iter_i, iter_j;
	gpointer key_i, value_i, key_j, value_j;

	g_hash_table_iter_init(&iter_i, ht);
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
}

guint lookup_counter(GHashTable *ht, char *major, char *minor)
{
	GHashTable *sub;
	guint *val;

	sub = g_hash_table_lookup(ht, major);
	if (sub == NULL)
		return 0;

	val = g_hash_table_lookup(sub, minor);
	if (val == NULL)
		return 0;
	return *val;
}
