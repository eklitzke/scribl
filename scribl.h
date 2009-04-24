#ifndef __SCRIBL_H__
#define __SCRIBL_H__

#include <glib.h>

/**
 * A counter: keys are strings, values are doubles.
 *
 * This represents a counter in scribl. It's the basic unit that calling code
 * would use to keep track of statistics. All fields are considered private.
 */
struct scribl_counter {
	GHashTable *ht; /**< The hash table itself. */
	GMutex *lock; /**< Mutex used to lock access to ht. */
	GMutex *swlock; /**< NOT YET USED */
	char *name; /**< The name of the counter (NULL is ok). */
};

/**
 * Initialize scribl.
 *
 * This method must be called before calling any other methods exposed by
 * scribl. It must only be called once.
 */
void scribl_init(double wakeup_interval);

/**
 * Exit scribl.
 */
void scribl_exit(void);

/** Create a new counter.
 *
 * Boring stuff.
 */
struct scribl_counter* scribl_new_counter(const char *name);
void scribl_incr_counter(struct scribl_counter *counter, char *key, double incr);
double scribl_lookup_counter(struct scribl_counter *counter, char *key);
void scribl_replace_counter_ev(struct scribl_counter *counter);
void scribl_free_counter(struct scribl_counter *counter);

#endif /* __SCRIBL_H__ */
