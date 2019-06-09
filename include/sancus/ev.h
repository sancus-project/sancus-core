#ifndef __SANCUS_EV_H__
#define __SANCUS_EV_H__

struct ev_loop;
struct ev_io;

typedef double ev_tstamp;
typedef void (*ev_io_cb) (struct ev_loop *, struct ev_io *, int);

#if 0
struct ev_loop {
	int placeholder;
};
#endif

enum {
	EV_READ,
	EV_WRITE,
	EV_ERROR,
};

static inline ev_tstamp ev_now(struct ev_loop *UNUSED(loop))
{
	return 0.0;
}

struct ev_io {
	int fd;
	ev_io_cb cb;
};


static inline int ev_io_init(struct ev_io *w, ev_io_cb cb, int fd, unsigned UNUSED(mode))
{
	w->fd = fd;
	w->cb = cb;
	return -38 /*-ENOSYS*/;
}

static inline int ev_io_start(struct ev_loop *UNUSED(loop), struct ev_io *UNUSED(w))
{
	return -38 /*-ENOSYS*/;
}

static inline int ev_io_stop(struct ev_loop *UNUSED(loop), struct ev_io *UNUSED(w))
{
	return -38 /*-ENOSYS*/;
}

static inline int ev_is_active(struct ev_io *UNUSED(w))
{
	return 0;
}

#endif /* !__SANCUS_EV_H__ */
