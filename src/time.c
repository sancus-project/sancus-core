#include <sancus/common.h>
#include <sancus/time.h>

struct timespec sancus_time_new2(long sec, long ns)
{
	long d = ns / SEC_TO_NS(1); // seconds adjustment

	if (d > 0) {
		sec += d;
		ns -= SEC_TO_NS(d);
	} else if (ns < 0) {
		d = -d + 1;

		if (d != 1 || sec != 0) {
			sec -= d;
			ns += SEC_TO_NS(d);
		}
	}

	return TIMESPEC_INIT(sec, ns);
}
