#ifndef __SANCUS_TIME_H__
#define __SANCUS_TIME_H__

#include <sys/time.h>
#include <limits.h>

#define SEC_TO_NS(S) (1000L * SEC_TO_US(S))
#define SEC_TO_US(S) (1000L * SEC_TO_MS(S))
#define SEC_TO_MS(S) (1000L * (S))

#define MS_TO_NS(MS)  (1000L * MS_TO_US(MS))
#define MS_TO_US(MS)  (1000L * (MS))
#define MS_TO_SEC(MS) ((MS) / SEC_TO_MS(1))

#define US_TO_NS(US)  (1000L * (US))
#define US_TO_MS(US)  ((US) / MS_TO_US(1))
#define US_TO_SEC(US) ((US) / SEC_TO_US(1))

#define NS_TO_MS(NS)  ((NS) / MS_TO_NS(1))
#define NS_TO_US(NS)  ((NS) / MS_TO_US(1))
#define NS_TO_SEC(NS) ((NS) / SEC_TO_NS(1))

#define NS_TO_MS_ROUNDED(NS) (((NS) + (MS_TO_NS(1)/2)) / MS_TO_NS(1))

#define TIMESPEC_INIT(S, NS)    ((struct timespec) { (S), (NS) })
#define TIMESPEC_INIT_MS(S, MS) TIMESPEC_INIT((S), MS_TO_NS(MS))

#define TIMESPEC_FMT    "%s%ld.%09ld"
#define TIMESPEC_FMT_MS "%s%ld.%03ld"

#define TIMESPEC_SPLIT(A) \
	(A)->tv_nsec < 0 ? "-" : "", \
	(A)->tv_sec, \
	(A)->tv_nsec < 0 ? -(A)->tv_nsec : (A)->tv_nsec

#define TIMESPEC_SPLIT_MS(A) \
	(A)->tv_nsec < 0 ? "-" : "", \
	(A)->tv_sec, \
	NS_TO_MS((A)->tv_nsec < 0 ? -(A)->tv_nsec : (A)->tv_nsec)

/* sancus_time_fp_to_ts converts a time duration from double to timespec */
static inline struct timespec sancus_time_fp_to_ts(double d)
{
	struct timespec t = {
		.tv_sec = d / 1L,
	};

	d -= t.tv_sec;
	/* shift to nanoseconds */
	d *= 1000000000.;
	/* rounding */
	d += .5;

	t.tv_nsec = d / 1L;
	return t;
}

/* sancus_time_fp_to_ms converts a time duration from double to milliseconds */
static inline long sancus_time_fp_to_ms(double d)
{
	/* shift to milliseconds */
	d *= 1000.;
	/* rounding */
	d += .5;

	return d / 1L;
}

/* sancus_time_ts_to_ms converts a time duration from timespec to milliseconds */
static inline long sancus_time_ts_to_ms(const struct timespec *ts)
{
	long ms;

	if (ts->tv_sec <= (time_t)(LONG_MIN/SEC_TO_MS(1)))
		ms = LONG_MIN;
	else if (ts->tv_sec >= (time_t)(LONG_MAX/SEC_TO_MS(1)))
		ms = LONG_MAX;
	else if (ts->tv_sec < 0)
		ms = SEC_TO_MS(ts->tv_sec) - NS_TO_MS_ROUNDED(ts->tv_nsec);
	else
		ms = SEC_TO_MS(ts->tv_sec) + NS_TO_MS_ROUNDED(ts->tv_nsec);
	return ms;
}

/* sancus_time_ts_to_fs converts a time duration from timespec to double */
static inline double sancus_time_ts_to_fp(const struct timespec *t)
{
	double d = t->tv_nsec;
	d /= 1000000000.;
	d += t->tv_sec;
	return d;
}

/* -A */
static inline struct timespec sancus_time_neg(struct timespec a)
{
	if (a.tv_sec == 0)
		a.tv_nsec = -a.tv_nsec;
	else
		a.tv_sec = -a.tv_sec;

	return a;
}

static inline int sancus_time_cmp(const struct timespec *a, const struct timespec *b)
{
	int ret = a->tv_sec - b->tv_sec;
	if (ret == 0) {
		if (a->tv_sec < 0)
			ret = b->tv_nsec - a->tv_nsec;
		else
			ret = a->tv_nsec - b->tv_nsec;
	}
	return ret;
}

/* A == B */
static inline int sancus_time_is_eq(const struct timespec *a, const struct timespec *b)
{
	return (a->tv_sec == b->tv_sec) && (a->tv_nsec == b->tv_nsec);
}

/* A == 0 */
static inline int sancus_time_is_zero(const struct timespec *a)
{
	return (a->tv_sec == 0) && (a->tv_nsec == 0);
}

/* A < B */
static inline int sancus_time_is_lt(const struct timespec *a, const struct timespec *b)
{
	return sancus_time_cmp(a, b) < 0;
}

/* A > B */
static inline int sancus_time_is_gt(const struct timespec *a, const struct timespec *b)
{
	return sancus_time_cmp(a, b) > 0;
}

/* turns timespec into canonical format, and returns
 * if the result is negative, positive or zero
 */
static inline int sancus_time_fix(struct timespec *ts)
{
	long d = ts->tv_nsec / SEC_TO_NS(1);

	if (d > 0) {
		ts->tv_sec += d;
		ts->tv_nsec -= SEC_TO_NS(d);
	} else if (ts->tv_nsec < 0) {
		d = -d + 1;

		if (d != 1 || ts->tv_sec != 0) {
			ts->tv_sec -= d;
			ts->tv_nsec += SEC_TO_NS(d);
		}
	}

	if (ts->tv_sec < 0 || ts->tv_nsec < 0)
		return -1;
	else if (ts->tv_sec == 0 && ts->tv_nsec == 0)
		return 0;
	else
		return 1;
}

/* create canonical timespec */
static inline struct timespec sancus_time_new(long sec, long ns)
{
	struct timespec ts = { sec, ns };
	sancus_time_fix(&ts);
	return ts;
}

/* A += B */
int sancus_time__add(struct timespec *a, const struct timespec *b);

/* A + B */
struct timespec sancus_time_add(const struct timespec *a, const struct timespec *b);

/* A - B */
static inline struct timespec sancus_time_sub(const struct timespec *a, const struct timespec *b)
{
	struct timespec t = sancus_time_neg(*b);
	return sancus_time_add(a, &t);
}

/* NOW > SINCE ? NOW - SINCE : 0 */
static inline struct timespec sancus_time_elapsed(const struct timespec *now,
						  const struct timespec *since)
{
	if (sancus_time_is_gt(now, since)) {
		return sancus_time_sub(now, since);
	}
	return TIMESPEC_INIT(0, 0);
}


/* UNTIL < NOW ? UNTIL - NOW : 0 */
static inline struct timespec sancus_time_left(const struct timespec *now,
					       const struct timespec *until)
{
	if (sancus_time_is_gt(until, now)) {
		return sancus_time_sub(until, now);
	}
	return TIMESPEC_INIT(0, 0);
}

#endif /* !__SANCUS_TIME_H__ */
