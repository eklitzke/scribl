#include "scribl.h"
#include <stdio.h>

gpointer thread_worker(gpointer data);

/* Test using a counter without threads */
void test_simple()
{
	struct scribl_counter *counter;
	
	scribl_init();
	counter = scribl_new_counter();
	scribl_incr_counter(counter, "foo", "bar");
	scribl_incr_counter(counter, "foo", "baz");
	scribl_incr_counter(counter, "foo", "bar");
	printf("foo.bar = %d\n", scribl_lookup_counter(counter, "foo", "bar"));
	printf("foo.baz = %d\n", scribl_lookup_counter(counter, "foo", "baz"));
	printf("foo.bad = %d\n", scribl_lookup_counter(counter, "foo", "bad"));
	scribl_free_counter(counter);
}

/* Test using a counter with threads */
void test_two_threads()
{
	struct scribl_counter *counter;
	guint *val;
	GThread *thd1, *thd2;

	scribl_init();
	counter = scribl_new_counter();
	thd1 = g_thread_create(thread_worker, counter, TRUE, NULL);
	thd2 = g_thread_create(thread_worker, counter, TRUE, NULL);
	val = g_thread_join(thd1);
	printf("thd1 exited with val = %d\n", *val);
	val = g_thread_join(thd2);
	printf("thd2 exited with val = %d\n", *val);
}

gpointer thread_worker(gpointer data)
{
	int i;
	guint *val;
	struct scribl_counter *counter;

	counter = (struct scribl_counter *) data;
	for (i = 0; i < 500000; i++) {
		scribl_incr_counter(counter, "foo", "bar");
	}
	val = g_slice_alloc(sizeof(guint));
	*val = scribl_lookup_counter(counter, "foo", "bar");
	g_thread_exit(val);
}

int main()
{
	test_two_threads();
	return 0;
}
