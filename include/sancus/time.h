#ifndef __SANCUS_TIME_H__
#define __SANCUS_TIME_H__

#include <sys/time.h>

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

#define TIMESPEC_INIT(S, NS)    ((struct timespec) { (S), (NS) })
#define TIMESPEC_INIT_MS(S, MS) TIMESPEC_INIT((S), MS_TO_NS(MS))

#define TIMESPEC_FMT    "%s%ld.%09ld"
#define TIMESPEC_FMT_MS "%s%ld.%06ld"

#define TIMESPEC_SPLIT(A) \
	(A)->tv_nsec < 0 ? "-" : "", \
	(A)->tv_sec, \
	(A)->tv_nsec < 0 ? -(A)->tv_nsec : (A)->tv_nsec

#define TIMESPEC_SPLIT_MS(A) \
	(A)->tv_nsec < 0 ? "-" : "", \
	(A)->tv_sec, \
	NS_TO_MS((A)->tv_nsec < 0 ? -(A)->tv_nsec : (A)->tv_nsec)

static inline void sancus_time_fix(struct timespec *a)
{
	long ns = a->tv_nsec;
	long d = ns / SEC_TO_NS(1);

	if (d > 0) {
		a->tv_sec += d;
		a->tv_nsec -= SEC_TO_NS(d);
	} else if (ns < 0) {
		d = -d + 1;

		if (unlikely(d == 1 && a->tv_sec == 0)) {
			/* a > -1.0 && a < 0 */
			;
		} else {
			a->tv_sec -= d;
			a->tv_nsec += SEC_TO_NS(d);
		}
	}
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

/* A += B */
static inline void sancus_time_add(struct timespec *a,
				   struct timespec b)
{
	if (sancus_time_is_zero(&b)) {
		; // B == 0
	} else if (sancus_time_is_zero(a)) {
		*a = b; // A == 0
	} else {
		a->tv_sec += b.tv_sec;
		a->tv_nsec += b.tv_nsec;
		sancus_time_fix(a);
	}
}

/* A -= B */
static inline void sancus_time_sub(struct timespec *a,
				   struct timespec b)
{
	if (!sancus_time_is_zero(&b))
		sancus_time_add(a, sancus_time_neg(b));
}

/* NOW > SINCE ? NOW - SINCE : 0 */
static inline struct timespec sancus_time_elapsed(const struct timespec *now,
						  const struct timespec *since)
{
	if (sancus_time_is_gt(now, since)) {
		struct timespec ret = *now;
		sancus_time_sub(&ret, *since);
		return ret;
	}
	return TIMESPEC_INIT(0, 0);
}


/* UNTIL < NOW ? UNTIL - NOW : 0 */
static inline struct timespec sancus_time_left(const struct timespec *now,
					       const struct timespec *until)
{
	return sancus_time_elapsed(until, now);
}

#endif /* !__SANCUS_TIME_H__ */
