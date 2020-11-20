#include "ev/private.h"

#include <sancus/ev/fd.h>

/*
 * unsafe helpers
 */
static inline int sancus_watcher__fd_start(struct sancus_fd_watcher *w,
					   struct sancus_ev_loop *loop)
{
	struct sancus_watcher *w0 = sancus_watcher_from_fd(w);
	int err;

	if (sancus_watcher_is_active(w0))
		err = 0;
	else if (unlikely(w == NULL || w->fd < 0 || w->cb == NULL))
		err = -EINVAL;
	else {
		sancus_list_del(&w0->watcher);

		err = ev_epoll_add(&loop->epoll, w);

		if (err)
			sancus_list_append(&loop->watchers, &w0->watcher);
		else
			w0->flags |= SANCUS_WATCHER_IS_ACTIVE;
	}

	return err;
}

static inline int sancus_watcher__fd_stop(struct sancus_fd_watcher *w,
					  struct sancus_ev_loop *loop)
{
	struct sancus_watcher *w0 = sancus_watcher_from_fd(w);
	int err;

	if (unlikely(w == NULL))
		err = -EINVAL;
	else if (!sancus_watcher_is_active(w0))
		err = 0;
	else {
		err = ev_epoll_del(&loop->epoll, w);

		if (!err) {
			sancus_list_del(&w0->watcher);
			sancus_list_append(&loop->watchers, &w0->watcher);
			w0->flags &= ~SANCUS_WATCHER_IS_ACTIVE;
		}
	}

	return err;
}

/*
 * entrypoints
 */
int sancus_watcher_fd_start(struct sancus_fd_watcher *w, struct sancus_ev_loop *loop1)
{
	struct sancus_ev_loop *loop = loop1;
	struct sancus_watcher *w0 = sancus_watcher_from_fd(w);

	int err = sancus_watcher_attach2(w0, &loop);

	if (!err && loop_lock(loop)) {
		err = sancus_watcher__fd_start(w, loop);
		loop_unlock(loop);
	}

	return err;
}

int sancus_watcher_fd_stop(struct sancus_fd_watcher *w)
{
	struct sancus_watcher *w0 = sancus_watcher_from_fd(w);
	if (likely(w0 != NULL)) {
		int err = 0;

		if (sancus_watcher_is_active(w0)) {
			struct sancus_ev_loop *loop;

			loop = sancus_watcher_get_loop(w0);
			if (loop_lock(loop)) {
				err = sancus_watcher__fd_stop(w, loop);
				loop_unlock(loop);
			} else {
				goto fail_invalid;
			}
		}

		return err;
	}

fail_invalid:
	return -EINVAL;
}
