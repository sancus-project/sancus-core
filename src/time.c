#include <sancus/common.h>
#include <sancus/time.h>

#include <stdio.h>

/* A - B, A > B */
static inline struct timespec substract(struct timespec a, struct timespec b)
{
	long sec = a.tv_sec - b.tv_sec - (a.tv_nsec<b.tv_nsec ? 1 : 0);
	long nsec = a.tv_nsec - b.tv_nsec + (a.tv_nsec<b.tv_nsec ? SEC_TO_NS(1) : 0);

	return sancus_time_new(sec, nsec);
}

int sancus_time__add(struct timespec *a, const struct timespec *b)
{
	*a = sancus_time_add(a, b);

	if (a->tv_sec < 0 || a->tv_nsec < 0)
		return -1;
	else if (a->tv_sec == 0 && a->tv_nsec == 0)
		return 0;
	else
		return 1;
}

struct timespec sancus_time_add(const struct timespec *a, const struct timespec *b)
{
	struct timespec zero = TIMESPEC_INIT(0, 0);
	int ca = sancus_time_cmp(a, &zero);
	int cb = sancus_time_cmp(b, &zero);

	if (ca == 0) {
		return *b;
	} else if (cb == 0) {
		return *a;
	} else if (ca > 0 && cb > 0) {
		return sancus_time_new(a->tv_sec + b->tv_sec,
				       a->tv_nsec + b->tv_nsec);
	} else if (ca < 0) {
		struct timespec na = sancus_time_neg(*a);

		if (cb < 0) {
			struct timespec nb = sancus_time_neg(*b);
			return sancus_time_neg(sancus_time_add(&na, &nb));
		} else if (sancus_time_is_lt(b, &na)) {
			return sancus_time_neg(substract(na, *b));
		} else {
			return substract(*b, na);
		}
	} else {
		struct timespec nb = sancus_time_neg(*b);
		if (sancus_time_is_lt(a, &nb)) {
			return sancus_time_neg(substract(nb, *a));
		} else {
			return substract(*a, nb);
		}
	}
}
