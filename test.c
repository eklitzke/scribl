#include "scribl.h"
#include <stdio.h>
#include <unistd.h> /* sleep(3) */

int main(int argc, char **argv)
{
	int i;
	struct scribl_counter *counter;

	scribl_init(3);
	counter = scribl_new_counter("test");

	for (i = 0; i < 10; i++) {
		printf("incrementing foo:bar from main thread, i = %d\n", i);
		scribl_incr_counter(counter, "foo:bar", 1);
		sleep(1);
	}

	/* Not reached */
	scribl_free_counter(counter);
	scribl_exit();
	return 0;
}

