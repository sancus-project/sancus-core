#include <sancus/common.h>
#include <sancus/time.h>

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
