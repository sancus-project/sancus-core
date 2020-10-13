#include "ev/private.h"

#include <time.h>

/*
 * safe helpers
 */
static inline int loop_done(struct sancus_ev_loop *loop)
{
	int stop = 0;

	if (loop_lock(loop)) {
		/* the loop stops when empty or asked to */
		unsigned flags = LOOP_STOP_TEST;

		stop = !!(flags & loop->flags);
		if (stop)
			loop->flags &= ~LOOP_STOP_CLEAR;

		loop_unlock(loop);
	}

	return stop;
}

/*
 *
 */
int sancus_ev_init(struct sancus_ev_loop *loop)
{
	int err;

	if (!loop)
		return -EINVAL;

	*loop = (struct sancus_ev_loop) {
		.mu = PTHREAD_MUTEX_INITIALIZER,

		.epoll = {
			.fd = -1,
		},
	};

	sancus_list_init(&loop->watchers);

	if ((err = sancus_ev_now_update(loop, NULL)) < 0)
		goto fail_now_update;
	if ((err = ev_epoll_init(&loop->epoll)) < 0)
		goto fail_epoll_init;

	return 0;

fail_epoll_init:
fail_now_update:
	return err;
}

int sancus_ev_finish(struct sancus_ev_loop *loop)
{
	int err;

	if (!loop_lock(loop))
		return -EINVAL;

	err = ev_epoll_finish(&loop->epoll);
	loop_unlock(loop);

	return err;
}

int sancus_ev_run_once(struct sancus_ev_loop *loop, unsigned wait)
{
	int rc;
	unsigned flags;

	if (!loop_lock(loop))
		return -EINVAL;

	flags = loop->flags;
	flags &= ~LOOP_ACTIVE;
	flags |= LOOP_RUNNING;
	loop->flags = flags;

	loop->count = 0;

	rc = ev_epoll_run_once(loop, wait, MAX_EVENTS);
	if (rc < 0)
		goto done;

	rc = loop->count;
	if (rc == 0) {
		/*
		 * update timestamp as it
		 * wasn't done before
		 */
		rc = loop__gettime(loop, NULL);
	}

done:
	flags = loop->flags;
	if (flags&LOOP_ACTIVE)
		flags &= ~LOOP_EMPTY;
	else
		flags |= LOOP_EMPTY;
	loop->flags = flags & ~LOOP_RUNNING;

	loop_unlock(loop);

	return rc;
}

int sancus_ev_run(struct sancus_ev_loop *loop, unsigned wait)
{
	int rc;

	if (loop == NULL)
		return -EINVAL;

	do {
		rc = sancus_ev_run_once(loop, wait);

		if (rc < 0)
			break;
	} while (!loop_done(loop));

	return rc;
}

int sancus_ev_stop(struct sancus_ev_loop *loop)
{
	if (!loop_lock(loop))
		return -EINVAL;

	loop->flags |= LOOP_MUST_STOP;
	loop_unlock(loop);

	return 0;
}

int sancus_ev_abort(struct sancus_ev_loop *loop)
{
	if (!loop_lock(loop))
		return -EINVAL;

	loop->flags |= LOOP_MUST_STOP;
	loop_unlock(loop);
	return 0;
}

int sancus_ev_now_update(struct sancus_ev_loop *loop, struct timespec *out)
{
	int err;

	if (!loop_lock(loop))
		return -EINVAL;

	err = loop__gettime(loop, out);
	loop_unlock(loop);

	return err;
}
