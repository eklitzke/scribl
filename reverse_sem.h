#ifndef __REVERSE_GSEM_H__
#define __REVERSE_GSEM_H__

#include <glib.h>

struct _GReverseSemaphore
{
	guint value;
	GMutex *access;
	GCond *cond;
};

typedef struct _GReverseSemaphore GReverseSemaphore;

GReverseSemaphore* g_reverse_semaphore_create();
void g_reverse_semaphore_up(GReverseSemaphore *sem);
void g_reverse_semaphore_down(GReverseSemaphore *sem);
void g_reverse_semaphore_destroy(GReverseSemaphore *sem);

#endif /* __REVERSE_GSEM_H__ */
