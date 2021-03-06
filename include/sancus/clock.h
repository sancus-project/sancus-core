#ifndef __SANCUS_CLOCK_H__
#define __SANCUS_CLOCK_H__

#include <sancus/time.h>
#include <time.h>

struct sancus_clock {
	int (*f) (void *, struct timespec *);
	void *data;
};

int sancus_now(struct timespec *);
void sancus_set_now_clock(struct sancus_clock *);

static inline struct timespec sancus_now_ts(void)
{
	struct timespec ts;
	sancus_now(&ts);
	return ts;
}

static inline double sancus_now_fp(void)
{
	struct timespec ts;
	sancus_now(&ts);
	return sancus_time_ts_to_fp(&ts);
}

static inline int sancus_gettime(struct timespec *ts)
{
	return ts != NULL ? clock_gettime(CLOCK_REALTIME, ts) : 0;
}

static inline int sancus_clock_gettime(struct sancus_clock *clk, struct timespec *ts)
{
	int err;

	if (clk != NULL && clk->f != NULL)
		err = clk->f(clk->data, ts);
	else
		err = sancus_gettime(ts);

	if (err != 0 && ts != NULL)
		*ts = (struct timespec) { 0, 0};

	return err;
}

#endif /* !__SANCUS_CLOCK_H__ */
