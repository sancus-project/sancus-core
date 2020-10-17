#include <sancus/common.h>
#include <sancus/time.h>

int sancus_time_fix(struct timespec *ts)
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

static int time_add(struct timespec *a, const struct timespec *b)
{
	if ((a->tv_sec < 0) ^ (b->tv_sec < 0)) {
		a->tv_sec += b->tv_sec;
		a->tv_nsec -= b->tv_nsec;

		if (a->tv_nsec < 0) {
			long d = 1 - a->tv_nsec / SEC_TO_NS(1);

			a->tv_nsec += SEC_TO_NS(d);

			if (a->tv_sec == -d) {
				a->tv_sec = 0;
				a->tv_nsec *= -1;
			} else if (a->tv_sec >= d) {
				a->tv_sec -= d;
			} else {
				a->tv_sec += d;
			}
		}

	} else {
		a->tv_sec += b->tv_sec;
		a->tv_nsec += b->tv_nsec;
	}

	return sancus_time_fix(a);
}

int sancus_time_add(struct timespec *a, const struct timespec *b)
{
	return time_add(a, b);
}

int sancus_time_sub(struct timespec *a, const struct timespec *b)
{
	struct timespec nb = sancus_time_neg(*b);
	return time_add(a, &nb);
}

struct timespec sancus_time_add2(const struct timespec *a, const struct timespec *b)
{
	struct timespec ts = *a;
	time_add(&ts, b);
	return ts;
}

struct timespec sancus_time_sub2(const struct timespec *a, const struct timespec *b)
{
	struct timespec ts = *a, nb = sancus_time_neg(*b);
	time_add(&ts, &nb);
	return ts;
}
