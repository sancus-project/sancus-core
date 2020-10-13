#ifndef __SANCUS_EV_FD_H__
#define __SANCUS_EV_FD_H__

#include <sancus/ev/watcher.h>

struct sancus_fd_watcher {
	struct sancus_watcher watcher;

	int fd;

	int (*cb) (struct sancus_fd_watcher *, unsigned, void *);
};

/*
 * convert
 */
#define sancus_watcher_from_fd(P) ((P) != NULL ? &(P)->watcher : NULL)
#define sancus_watcher__to_fd(P) ((P) != NULL ? container_of(P, struct sancus_fd_watcher, watcher) : NULL)

static inline int sancus_watcher_to_fd(struct sancus_watcher *w,
				       struct sancus_fd_watcher **out)
{
	if (w == NULL || w->type != SANCUS_W_FD)
		return 0;

	if (out)
		*out = sancus_watcher__to_fd(w);

	return 1;
}

/*
 * methods
 */
static inline int sancus_watcher_fd_attach(struct sancus_fd_watcher *w, struct sancus_ev_loop *loop)
{
	struct sancus_watcher *w0 = sancus_watcher_from_fd(w);
	if (w0)
		return sancus_watcher_attach(w0, loop);
	return -EINVAL;
}

static inline int sancus_watcher_fd_detach(struct sancus_fd_watcher *w)
{
	struct sancus_watcher *w0 = sancus_watcher_from_fd(w);
	if (w0)
		return sancus_watcher_detach(w0);
	return -EINVAL;
}

int sancus_watcher_fd_start(struct sancus_fd_watcher *, struct sancus_ev_loop *);
int sancus_watcher_fd_stop(struct sancus_fd_watcher *);

#endif /* !__SANCUS_EV_FD_H__ */
