#include "ev/private.h"

#include <sancus/ev/fd.h>
#include <sancus/fd.h>

#include <sys/epoll.h>

/*
 * struct sancus_fd_watcher
 */
int ev_epoll_add(struct sancus_ev_epoll *epoll, struct sancus_fd_watcher *w)
{
	struct sancus_watcher *w0 = sancus_watcher_from_fd(w);
	struct epoll_event ev = {
		.events = w->events,
		.data = {
			.fd = w->fd,
		},
	};

	if (epoll_ctl(epoll->fd, EPOLL_CTL_ADD, w->fd, &ev) < 0)
		return -errno;

	sancus_list_append(&epoll->watchers,  &w0->watcher);
	return 0;
}

#if 0
int ev_epoll_mod(struct sancus_ev_epoll *epoll, struct sancus_fd_watcher *w, unsigned events);
int ev_epoll_del(struct sancus_ev_epoll *epoll, struct sancus_fd_watcher *w);
#endif

/*
 * struct sancus_ev_epoll
 */
int ev_epoll_init(struct sancus_ev_epoll *epoll)
{
	int fd, err;

	if (unlikely(epoll == NULL))
		return -EINVAL;
	if (unlikely(epoll->fd >= 0))
		return -EBUSY;

	sancus_list_init(&epoll->watchers);

	fd = epoll_create1(EPOLL_CLOEXEC);
	err = fd < 0 ? -errno : 0;

	epoll->fd = fd;
	return err;
}

int ev_epoll_finish(struct sancus_ev_epoll *epoll)
{
	if (unlikely(epoll == NULL))
		return -EINVAL;

	sancus_list_foreach2(&epoll->watchers, item, next) {
		struct sancus_watcher *w0 = container_of(item, struct sancus_watcher, watcher);

		sancus_watcher__finish(w0, 0);
	}

	if (!sancus_list_is_empty(&epoll->watchers))
		return -EBUSY;

	if (epoll->fd >= 0) {
		sancus_close(epoll->fd);
		epoll->fd = -1;
	}

	return 0;
}

/*
 * sancus_ev_loop
 */
static inline int ev_epoll_run_event(struct sancus_ev_loop *loop,
				     struct epoll_event *ev,
				     struct sancus_list *watchers)
{
	sancus_list_foreach2(watchers, item, next) {
		struct sancus_watcher *w0 = container_of(item, struct sancus_watcher, watcher);
		struct sancus_fd_watcher *w = sancus_watcher__to_fd(w0);

		if (ev->data.fd == w->fd) {
			int rc = -EINVAL;

			loop__gettime(loop, NULL);
			loop->count++;

			if (likely(w->cb != NULL)) {
				loop_unlock(loop);
				rc = w->cb(w, ev->events, w0->data);
				loop_lock(loop);
			}

			if (rc < 0)
				sancus_watcher__fd_finish(w, rc);

			return 1;
		}
	}

	return 0;
}

int ev_epoll_run_once(struct sancus_ev_loop *loop,
		      unsigned wait, unsigned nevents)
{
	if (unlikely(loop == NULL))
		return -EINVAL;

	if (!sancus_list_is_empty(&loop->epoll.watchers)) {
		struct epoll_event events[nevents];
		ssize_t nfds;

		loop->flags |= LOOP_ACTIVE;

		nfds = epoll_wait(loop->epoll.fd, events,
				  ARRAY_SIZE(events),
				  wait);

		if (nfds < 0)
			return nfds;

		for (ssize_t i = 0; i < nfds; i++) {
			ev_epoll_run_event(loop, &events[i], &loop->epoll.watchers);
		}
	}

	return 0;
}
