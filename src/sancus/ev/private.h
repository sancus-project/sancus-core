#include <sancus/common.h>

#include <sancus/ev2.h>

#include <sancus/bit.h>
#include <errno.h>

enum {
	/*
	 * bit flags
	 */
	LOOP_EMPTY_BIT,
	LOOP_ACTIVE_BIT,
	LOOP_RUNNING_BIT,

	/* self-clearnig flags */
	LOOP_MUST_STOP_BIT,
	LOOP_MUST_ABORT_BIT,

	/*
	 * masks
	 */
	LOOP_EMPTY  = BIT(LOOP_EMPTY_BIT),
	LOOP_ACTIVE = BIT(LOOP_ACTIVE_BIT),
	LOOP_RUNNING = BIT(LOOP_RUNNING_BIT),

	LOOP_MUST_STOP  = BIT(LOOP_MUST_STOP_BIT),
	LOOP_MUST_ABORT = BIT(LOOP_MUST_ABORT_BIT),

	LOOP_STOP_TEST  = LOOP_MUST_STOP|LOOP_MUST_ABORT|LOOP_EMPTY,
	LOOP_STOP_CLEAR = LOOP_MUST_STOP|LOOP_MUST_ABORT,
};

/*
 * safe helpers
 */
static inline int loop_lock(struct sancus_ev_loop *loop)
{
	if (loop) {
		pthread_mutex_lock(&loop->mu);
		return 1;
	}
	return 0;
}

static inline void loop_unlock(struct sancus_ev_loop *loop)
{
	if (loop)
		pthread_mutex_unlock(&loop->mu);
}

/*
 * unsafe helpers
 */
static inline int loop__gettime(struct sancus_ev_loop *loop,
				struct timespec *ts)
{
	struct timespec now;

	if (clock_gettime(CLOCK_MONOTONIC_RAW, &now) < 0)
		return -errno;
	else if (ts)
		*ts = now;

	loop->now = now;
	return 0;
}
