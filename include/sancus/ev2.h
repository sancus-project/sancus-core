#ifndef __SANCUS_EV_H__
#define __SANCUS_EV_H__

#include <sancus/list.h>
#include <pthread.h>

struct sancus_ev_loop {
	pthread_mutex_t mu;

	unsigned flags;
	unsigned count;

	struct timespec now;

	/* inactive watchers */
	struct sancus_list watchers;
};

int sancus_ev_init(struct sancus_ev_loop *);
int sancus_ev_finish(struct sancus_ev_loop *);
int sancus_ev_run_once(struct sancus_ev_loop *, unsigned wait);
int sancus_ev_run(struct sancus_ev_loop *, unsigned wait);

int sancus_ev_stop(struct sancus_ev_loop *);
int sancus_ev_abort(struct sancus_ev_loop *);

int sancus_ev_now_update(struct sancus_ev_loop *, struct timespec *out);

static inline struct timespec sancus_ev_now(const struct sancus_ev_loop *loop)
{
	if (loop != NULL)
		return loop->now;

	return (struct timespec) {0, 0};
}

#endif /* !__SANCUS_EV_H__ */
