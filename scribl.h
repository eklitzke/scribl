#ifndef __SCRIBL_H__
#define __SCRIBL_H__

#include <glib.h>

struct scribl_counter {
	GHashTable *ht;
	GMutex *lock;
};

void scribl_init();
struct scribl_counter* scribl_new_counter();
void scribl_incr_counter(struct scribl_counter *counter, char *major, char *minor);
guint scribl_lookup_counter(struct scribl_counter *counter, char *major, char *minor);
void scribl_free_counter(struct scribl_counter *counter);

#endif /* __SCRIBL_H__ */
