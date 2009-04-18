#include "scribl.h"
#include <stdio.h>
#include <unistd.h> /* sleep(3) */

int main(int argc, char **argv)
{
	struct scribl_counter *counter;

	scribl_init();
	counter = scribl_new_counter();

	while (1) {
		puts("incrementing foo:bar from main thread");
		scribl_incr_counter(counter, "foo:bar");
		sleep(1);
	}

	/* Not reached */
	scribl_free_counter(counter);
	scribl_exit();
}

