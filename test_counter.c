#include "scribl.h"
#include <stdio.h>

int main()
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
	return 0;
}
