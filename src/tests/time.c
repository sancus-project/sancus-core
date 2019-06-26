#include <sancus/common.h>
#include <sancus/time.h>

#include <stdio.h>
#include <stdlib.h>

#if 1
#define pr_info(...) fprintf(stdout, __VA_ARGS__)
#else
#define pr_info(...) do { } while(0)
#endif
#define pr_err(...)  fprintf(stderr, __VA_ARGS__)

static int test_eq__long(const char *fn, long v0, long v1, long v2)
{
	int err = 0;

	if (v1 == v2) {
		pr_info("%s(%ld) -> %ld\n", fn, v0, v1);
	} else {
		pr_err("%s(%ld) -> %ld expected:%ld\n", fn, v0, v1, v2);
		err = 1;
	}
	return err;
}
#define test_eq_long(F,X,Y) test_eq__long(#F, X, F(X), Y)

static int test_eq__time(const char *fn, struct timespec v0,
			 struct timespec v1, struct timespec v2)
{
	int err = 0;

	if (sancus_time_is_eq(&v1, &v2)) {
		pr_info("%s(%ld,%ld) -> " TIMESPEC_FMT "\n",
			fn, v0.tv_sec, v0.tv_nsec,
			TIMESPEC_SPLIT(&v1));
	} else {
		pr_err("%s(%ld,%ld) -> " TIMESPEC_FMT \
		       " expected:" TIMESPEC_FMT "\n",
		       fn, v0.tv_sec, v0.tv_nsec,
		       TIMESPEC_SPLIT(&v1),
		       TIMESPEC_SPLIT(&v2));
		err = 1;
	}
	return err;
}
#define test_eq_time(F,A0,B0,A1,B1) test_eq__time(#F, (struct timespec){A0,B0}, \
						  F(A0,B0), \
						  (struct timespec){A1,B1})

static int test_time_new(struct timespec v0, struct timespec v2)
{
	const char *fn = "sancus_time_fix";
	struct timespec v1 = v0;
	int err = 0;

	sancus_time_fix(&v1);

	if (sancus_time_is_eq(&v1, &v2)) {
		pr_info("%s(%ld,%ld) -> " TIMESPEC_FMT "\n",
			fn, v0.tv_sec, v0.tv_nsec,
			TIMESPEC_SPLIT(&v1));
	} else {
		pr_err("%s(%ld,%ld) -> " TIMESPEC_FMT " expected:" TIMESPEC_FMT "\n",
		       fn, v0.tv_sec, v0.tv_nsec,
		       TIMESPEC_SPLIT(&v1),
		       TIMESPEC_SPLIT(&v2));
		err = 1;
	}

	return err;
}

/*
 * t0: F(v0) == v1 == v2
 */
static int test__t0(const char *fn, struct timespec v0, struct timespec v1, struct timespec v2)
{
	int err = 0;

	if (sancus_time_is_eq(&v1, &v2)) {
		pr_info("%s(" TIMESPEC_FMT ") -> " TIMESPEC_FMT "\n",
			fn, TIMESPEC_SPLIT(&v0),
			TIMESPEC_SPLIT(&v1));
	} else {
		pr_err("%s(" TIMESPEC_FMT ") -> " TIMESPEC_FMT " expected:" TIMESPEC_FMT "\n",
		       fn, TIMESPEC_SPLIT(&v0),
		       TIMESPEC_SPLIT(&v1),
		       TIMESPEC_SPLIT(&v2));
		err = 1;
	}
	return err;
}

#define test_t0(F, T0, T2) test__t0("sancus_time_" #F, T0, sancus_time_ ##F(T0), T2)

/*
 * t1: f(a, b) == rc
 */
static int test__t1(const char *fn, struct timespec a, struct timespec b,
		    int r0, int r1)
{
	int err = 0;

	if (r0 == r1) {
		pr_info("%s(" TIMESPEC_FMT ", " TIMESPEC_FMT ") -> %d\n",
		       fn, TIMESPEC_SPLIT(&a), TIMESPEC_SPLIT(&b),
		       r0);
	} else {
		pr_err("%s(" TIMESPEC_FMT ", " TIMESPEC_FMT ") -> %d expected:%d\n",
		       fn, TIMESPEC_SPLIT(&a), TIMESPEC_SPLIT(&b),
		       r0, r1);

		err = 1;
	}
	return err;
}

#define test_t1(F, A, B, RC) test__t1(#F, A, B, F(&A, &B), RC)

/*
 * F(A, B) == C; C == D
 */
static struct timespec test_time__add(struct timespec a, struct timespec b)
{
	sancus_time_add(&a, &b);
	return a;
}

static struct timespec test_time__sub(struct timespec a, struct timespec b)
{
	sancus_time_sub(&a, &b);
	return a;
}

static struct timespec test_time__elapsed(struct timespec a, struct timespec b)
{
	return sancus_time_elapsed(&a, &b);
}

static struct timespec test_time__left(struct timespec a, struct timespec b)
{
	return sancus_time_left(&a, &b);
}

static int test__t2(const char *fn, struct timespec a, struct timespec b,
		    struct timespec c, struct timespec d)
{
	int err = 0;

	if (sancus_time_is_eq(&c, &d)) {
		pr_info("%s(" TIMESPEC_FMT ", " TIMESPEC_FMT ") -> " TIMESPEC_FMT "\n",
			fn, TIMESPEC_SPLIT(&a), TIMESPEC_SPLIT(&b),
			TIMESPEC_SPLIT(&c));
	} else {
		pr_err("%s(" TIMESPEC_FMT ", " TIMESPEC_FMT ") -> " TIMESPEC_FMT
		       " expected:" TIMESPEC_FMT "\n",
			fn, TIMESPEC_SPLIT(&a), TIMESPEC_SPLIT(&b),
			TIMESPEC_SPLIT(&c), TIMESPEC_SPLIT(&d));
		err = 1;
	}

	return err;
}

#define test_t2(F, A, B, C) test__t2("sancus_time_" #F, (A), (B), test_time__ ##F((A), (B)), (C))

/*
 */
#define T(S,MS) TIMESPEC_INIT_MS((S), (MS))

int main(int UNUSED(argc), char **UNUSED(argv))
{
	int err = 0;

	// conversions
	err += test_eq_time(TIMESPEC_INIT,    1,    0, 1,       0L);
	err += test_eq_time(TIMESPEC_INIT,    1,    2, 1,       2L);
	err += test_eq_time(TIMESPEC_INIT,    1, 2000, 1,    2000L);
	err += test_eq_time(TIMESPEC_INIT_MS, 1,    0, 1,       0L);
	err += test_eq_time(TIMESPEC_INIT_MS, 1,    2, 1, 2000000L);

	err += test_eq_long(SEC_TO_NS,       1, 1000000000L);
	err += test_eq_long(SEC_TO_US,       1,    1000000L);
	err += test_eq_long(SEC_TO_MS,       1,       1000L);

	err += test_eq_long(MS_TO_NS,        1,    1000000L);
	err += test_eq_long(MS_TO_US,        1,       1000L);
	err += test_eq_long(MS_TO_SEC,       1,          0L);
	err += test_eq_long(MS_TO_SEC,     999,          0L);
	err += test_eq_long(MS_TO_SEC,    1000,          1L);

	err += test_eq_long(US_TO_NS,        1,       1000L);

	err += test_eq_long(US_TO_MS,        1,          0L);
	err += test_eq_long(US_TO_MS,      999,          0L);
	err += test_eq_long(US_TO_MS,     1000,          1L);

	err += test_eq_long(US_TO_SEC,       1,          0L);
	err += test_eq_long(US_TO_SEC,     999,          0L);
	err += test_eq_long(US_TO_SEC,  999999,          0L);
	err += test_eq_long(US_TO_SEC, 1000000,          1L);

	err += test_eq_long(NS_TO_US,           1L, 0L);
	err += test_eq_long(NS_TO_US,         999L, 0L);
	err += test_eq_long(NS_TO_US,        1000L, 1L);
	err += test_eq_long(NS_TO_MS,           1L, 0L);
	err += test_eq_long(NS_TO_MS,         999L, 0L);
	err += test_eq_long(NS_TO_MS,      999999L, 0L);
	err += test_eq_long(NS_TO_MS,     1000000L, 1L);
	err += test_eq_long(NS_TO_SEC,          1L, 0L);
	err += test_eq_long(NS_TO_SEC,        999L, 0L);
	err += test_eq_long(NS_TO_SEC,     999999L, 0L);
	err += test_eq_long(NS_TO_SEC,  999999999L, 0L);
	err += test_eq_long(NS_TO_SEC, 1000000000L, 1L);

	// struct sanitisation
	err += test_time_new(T(0,   0), T(0,  0));
	err += test_time_new(T(1,  10), T(1, 10));
	err += test_time_new(T(1, 999), T(1,999));
	err += test_time_new(T(1,1000), T(2,  0));
	err += test_time_new(T(1,1001), T(2,  1));
	err += test_time_new(T(1,  -1), T(0,999));
	// negative
	err += test_t0(neg, T( 0,  0), T( 0,  0));
	err += test_t0(neg, T( 0,  1), T( 0, -1));
	err += test_t0(neg, T( 0, -1), T( 0,  1));
	err += test_t0(neg, T( 1,  0), T(-1,  0));
	err += test_t0(neg, T(-1,  0), T( 1,  0));
	err += test_t0(neg, T( 1,  2), T(-1,  2));
	err += test_t0(neg, T(-1,  2), T( 1,  2));

	// timespec compare
	err += test_t1(sancus_time_is_gt, T( 0, 2),   T( 0,   1), 1);
	err += test_t1(sancus_time_is_gt, T( 1, 0),   T( 0, 999), 1);
	err += test_t1(sancus_time_is_gt, T( 1, 0),   T( 1,   0), 0);
	err += test_t1(sancus_time_is_gt, T(-1, 0),   T(-2,   0), 1);
	err += test_t1(sancus_time_is_gt, T(-1, 100), T(-1, 200), 1);
	err += test_t1(sancus_time_is_gt, T( 0,-100), T( 0,-200), 1);
	err += test_t1(sancus_time_is_gt, T( 0,-200), T( 0,-100), 0);
	err += test_t1(sancus_time_is_gt, T( 0,-100), T( 0,-100), 0);

	// addition
	err += test_t2(add, T(0, 2), T(1, 997), T(1, 999));
	err += test_t2(add, T(0, 2), T(1, 998), T(2,   0));
	err += test_t2(add, T(0, 2), T(1, 999), T(2,   1));
	err += test_t2(add, T(0, -2), T(1, 1),  T(0, 999));
	err += test_t2(add, T(0, -2), T(0, 1),  T(0,  -1));
	err += test_t2(add, T(0, -2), T(0, 3),  T(0,   1));
	err += test_t2(add, T(-1, 200), T(0, 300),  T(0,-900));
	// subtraction
	err += test_t2(sub, T(0, 2), T(0, 1), T(0,1));
	err += test_t2(sub, T(0, 2), T(0, 2), T(0,0));
	err += test_t2(sub, T(0, 2), T(0, 3), T(0,-1));
	err += test_t2(sub, T(-1, 200), T(0, 300),  T(-1,500));

	err += test_t2(left, T(0,800), T(1,100), T(0,300));
	err += test_t2(left, T(1,100), T(1,100), T(0,0));
	err += test_t2(left, T(1,400), T(1,100), T(0,0));
	err += test_t2(elapsed, T(0,800), T(1,100), T(0,0));
	err += test_t2(elapsed, T(1,100), T(1,100), T(0,0));
	err += test_t2(elapsed, T(1,400), T(1,100), T(0,300));

	return err == 0 ? 0 : 1;
}
