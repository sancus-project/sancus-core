#include <sancus/common.h>
#include <sancus/clock.h>

static struct sancus_clock *backend;

void sancus_set_now_clock(struct sancus_clock *clk)
{
	backend = clk != NULL ? clk : NULL;
}

int sancus_now(struct timespec *ts)
{
	return sancus_clock_gettime(backend, ts);
}
