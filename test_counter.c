#include <glib.h>
#include "counter.c"
#include <stdio.h>

int main()
{
	GHashTable *ht = new_counter();
	incr_counter(ht, "foo", "bar");
	incr_counter(ht, "foo", "baz");
	incr_counter(ht, "foo", "bar");
	printf("foo.bar = %d\n", lookup_counter(ht, "foo", "bar"));
	printf("foo.baz = %d\n", lookup_counter(ht, "foo", "baz"));
	printf("foo.bad = %d\n", lookup_counter(ht, "foo", "bad"));
	free_counter(ht);
	return 0;
}
