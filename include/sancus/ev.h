#ifndef __SANCUS_EV_H__
#define __SANCUS_EV_H__

struct sancus_ev_loop;
struct sancus_ev_fd;

typedef double ev_tstamp;
typedef void (*sancus_ev_fd_cb) (struct sancus_ev_loop *, struct sancus_ev_fd *, int);

enum {
	SANCUS_EV_READ,
	SANCUS_EV_WRITE,
	SANCUS_EV_ERROR,
};

static inline ev_tstamp ev_now(struct sancus_ev_loop *UNUSED(loop))
{
	return 0.0;
}

struct sancus_ev_fd {
	int fd;
	sancus_ev_fd_cb cb;
};


static inline int sancus_ev_fd_init(struct sancus_ev_fd *w, sancus_ev_fd_cb cb, int fd, unsigned UNUSED(mode))
{
	w->fd = fd;
	w->cb = cb;
	return -38 /*-ENOSYS*/;
}

static inline int sancus_ev_fd_start(struct sancus_ev_loop *UNUSED(loop), struct sancus_ev_fd *UNUSED(w))
{
	return -38 /*-ENOSYS*/;
}

static inline int sancus_ev_fd_stop(struct sancus_ev_loop *UNUSED(loop), struct sancus_ev_fd *UNUSED(w))
{
	return -38 /*-ENOSYS*/;
}

static inline int sancus_ev_is_active(struct sancus_ev_fd *UNUSED(w))
{
	return 0;
}

#endif /* !__SANCUS_EV_H__ */
