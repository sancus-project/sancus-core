#include <sancus/common.h>

#include <sancus/ev2.h>
#include <sancus/ev/watcher.h>

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

	/*
	 * constants
	 */
	MAX_EVENTS = 16,
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

/*
 * sancus_watcher
 */
int sancus_watcher__init(struct sancus_watcher *w,
			 enum sancus_watcher_type type);

int sancus_watcher__finish(struct sancus_watcher *w, int err);

/*
 * sancus_fd_watcher
 */
struct sancus_fd_watcher;

int sancus_watcher__fd_finish(struct sancus_fd_watcher *w, int err);

/*
 * epoll
 */
int ev_epoll_init(struct sancus_ev_epoll *epoll);
int ev_epoll_finish(struct sancus_ev_epoll *epoll);

int ev_epoll_add(struct sancus_ev_epoll *, struct sancus_fd_watcher *);
int ev_epoll_mod(struct sancus_ev_epoll *, struct sancus_fd_watcher *, unsigned events);
int ev_epoll_del(struct sancus_ev_epoll *, struct sancus_fd_watcher *);

int ev_epoll_run_once(struct sancus_ev_loop *loop,
		      unsigned wait, unsigned nevents);
