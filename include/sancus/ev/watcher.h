#ifndef __SANCUS_EV_WATCHER_H__
#define __SANCUS_EV_WATCHER_H__

#include <sancus/bit.h>
#include <sancus/list.h>

#include <errno.h>

struct sancus_ev_loop;

enum {
	/* flags */
	SANCUS_WATCHER_IS_ACTIVE = BIT(1),
};

enum sancus_watcher_type {
	SANCUS_W_UNDEFINED,
	SANCUS_W_FD,
	SANCUS_W_SIGNAL,
	SANCUS_W_TIMER,
};

struct sancus_watcher {
	enum sancus_watcher_type type;
	void *data;

	struct sancus_ev_loop *loop;
	struct sancus_list watcher;

	unsigned flags;
};

/*
 * accessors
 */
static inline int sancus_watcher_is_active(struct sancus_watcher *w)
{
	if (w != NULL && w->loop != NULL)
		return !sancus_list_is_empty(&w->watcher) &&
			sancus_bit_has_flag(w->flags, SANCUS_WATCHER_IS_ACTIVE);
	return 0;
}

static inline struct sancus_ev_loop *sancus_watcher_get_loop(const struct sancus_watcher *w)
{
	return likely(w != NULL) ? w->loop : NULL;
}

static inline enum sancus_watcher_type sancus_watcher_get_type(const struct sancus_watcher *w)
{
	return likely(w != NULL) ? w->type : SANCUS_W_UNDEFINED;
}

static inline void *sancus_watcher_get_data(const struct sancus_watcher *w)
{
	return likely(w != NULL) ? w->data : NULL;
}

static inline int sancus_watcher_set_data(struct sancus_watcher *w, void *data)
{
	if (unlikely(w == NULL))
		return -EINVAL;

	w->data = data;
	return 0;
}

/*
 * methods
 */
int sancus_watcher_attach2(struct sancus_watcher *, struct sancus_ev_loop **);

static inline int sancus_watcher_attach(struct sancus_watcher *w,
					struct sancus_ev_loop *loop)
{
	struct sancus_ev_loop *loop1 = loop;

	return sancus_watcher_attach2(w, &loop1);
}

int sancus_watcher_detach(struct sancus_watcher *);

int sancus_watcher_start(struct sancus_watcher *,
			 struct sancus_ev_loop *);

int sancus_watcher_stop(struct sancus_watcher *);

#endif /* !__SANCUS_EV_WATCHER_H__ */
