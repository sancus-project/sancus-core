#include <sancus/common.h>
#include <sancus/logger.h>

#include <sancus/ev2.h>

static DECL_SANCUS_LOGGER(logger, TEST_NAME, SANCUS_LOG_VERBOSE|SANCUS_LOG_TRACE);

#define TRACE sancus_log_trace(&logger, NULL)

int main(int UNUSED(argc), char **UNUSED(argv))
{
	struct sancus_ev_loop loop;

	TRACE;

	sancus_ev_init(&loop);
	sancus_ev_run(&loop, 10);
	sancus_ev_finish(&loop);

	TRACE;

	return 0;
}
