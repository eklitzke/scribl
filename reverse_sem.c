#include "reverse_sem.h"
#include <glib.h>

GReverseSemaphore* g_reverse_semaphore_create()
{
	GReverseSemaphore *sem;
	sem = g_slice_alloc(sizeof(GReverseSemaphore));
	sem->value = 0;
	sem->access = g_mutex_new();
	sem->cond = g_cond_new();
	return sem;
}

void g_reverse_semaphore_up(GReverseSemaphore *sem)
{
	g_mutex_lock(sem->access);
	sem->value = sem->value + 1;
	g_mutex_unlock(sem->access);
}

void g_reverse_semaphore_down(GReverseSemaphore *sem)
{
	g_mutex_lock(sem->access);
	g_assert(sem->value > 0);
	sem->value = sem->value - 1;
	g_mutex_unlock(sem->access);
	g_cond_signal(sem->cond);
}

/* This waits for the semaphore, and returns it still locked */
void g_reverse_semaphore_destroy(GReverseSemaphore *sem)
{
	g_mutex_lock(sem->access);
	while (sem->value > 0)
		g_cond_wait(sem->cond, sem->access);
	g_mutex_unlock(sem->access);
	g_mutex_free(sem->access);
	g_cond_free(sem->cond);
	g_slice_free(GReverseSemaphore, sem);
}
