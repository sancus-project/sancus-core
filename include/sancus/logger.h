#ifndef __SANCUS_LOGGER_H__
#define __SANCUS_LOGGER_H__

#include <sancus/bit.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

enum sancus_log_level {
	SANCUS_LOG_ERR   = BIT(0),
	SANCUS_LOG_WARN  = BIT(1),
	SANCUS_LOG_INFO  = BIT(2),
	SANCUS_LOG_TRACE = BIT(3),
	SANCUS_LOG_DEBUG = BIT(4),
};

enum {
	SANCUS_LOG_NEXT  = BIT(8),

	SANCUS_LOG_QUIET   = SANCUS_LOG_ERR | SANCUS_LOG_WARN,
	SANCUS_LOG_NORMAL  = SANCUS_LOG_QUIET | SANCUS_LOG_INFO,
	SANCUS_LOG_VERBOSE = SANCUS_LOG_NORMAL | SANCUS_LOG_DEBUG,
};

struct sancus_logger {
	const char *prefix;
	unsigned mask;
};

static inline
void sancus_logger_set_prefix(struct sancus_logger *log,
			      const char *prefix)
{
	log->prefix = prefix;
}

static inline
void sancus_logger_set_mask(struct sancus_logger *log,
			    unsigned mask)
{
	log->mask = mask;
}

static inline
void sancus_logger_extend_mask(struct sancus_logger *log,
			       unsigned mask)
{
	log->mask = sancus_bit_cms(log->mask, mask, mask, 0);
}

static inline
void sancus_logger_unset_mask(struct sancus_logger *log,
			      unsigned mask)
{
	log->mask = sancus_bit_cms(log->mask, 0, mask, 0);
}

static inline
int sancus_logger_has_level(const struct sancus_logger *log,
			    unsigned mask)
{
	if (!mask || !!(mask&SANCUS_LOG_ERR))
		mask = 1;
	else if (log)
		mask &= log->mask;
	else
		mask &= SANCUS_LOG_NORMAL;

	return !!mask;
}

/*
 */
int sancus_logger__vprintf(const struct sancus_logger *log,
			   enum sancus_log_level level,
			   const char *func, unsigned line,
			   const char *fmt, va_list ap);

__attr_printf(5) static inline
int sancus_logger__printf(const struct sancus_logger *log,
			  enum sancus_log_level level,
			  const char *func, unsigned line,
			  const char *fmt, ...)
{
	va_list ap;
	int err;
	va_start(ap, fmt);
	err = sancus_logger__vprintf(log, level,
				     func, line,
				     fmt, ap);
	va_end(ap);
	return err;
}

/*
 */
int sancus_logger__vdumpf(const struct sancus_logger *log,
			  enum sancus_log_level level,
			  const char *func, size_t line,
			  const void *data, size_t data_len,
			  const char *fmt, va_list ap);

__attr_printf(7) static inline
int sancus_logger__dumpf(const struct sancus_logger *log,
			  enum sancus_log_level level,
			  const char *func, unsigned line,
			  const void *data, size_t data_len,
			  const char *fmt, ...)
{
	va_list ap;
	int err;
	va_start(ap, fmt);
	err = sancus_logger__vdumpf(log, level, func, line,
				    data, data_len,
				    fmt, ap);
	va_end(ap);
	return err;
}

/*
 */
#define sancus_log__error(D, ...)          sancus_logger__printf((D), SANCUS_LOG_ERR,   NULL, 0, __VA_ARGS__)
#define sancus_log__error_dump(D, ...)     sancus_logger__dumpf((D),  SANCUS_LOG_ERR,   NULL, 0, __VA_ARGS__)

#define sancus_log__warn(D, ...)           sancus_logger__printf((D), SANCUS_LOG_WARN,  NULL, 0, __VA_ARGS__)
#define sancus_log__warn_dump(D, ...)      sancus_logger__dumpf((D),  SANCUS_LOG_WARN,  NULL, 0, __VA_ARGS__)

#define sancus_log__info(D, ...)           sancus_logger__printf((D), SANCUS_LOG_INFO,  NULL, 0, __VA_ARGS__)
#define sancus_log__info_dump(D, ...)      sancus_logger__dumpf((D),  SANCUS_LOG_INFO,  NULL, 0, __VA_ARGS__)

#define sancus_log__trace(D, ...)          sancus_logger__printf((D), SANCUS_LOG_TRACE, __func__, __LINE__, __VA_ARGS__)
#define sancus_log__trace_dump(D, ...)     sancus_logger__dumpf((D),  SANCUS_LOG_TRACE, __func__, __LINE__, __VA_ARGS__)

#define sancus_log__debug(D, ...)          sancus_logger__printf((D), SANCUS_LOG_DEBUG, __func__, 0, __VA_ARGS__)
#define sancus_log__debug_dump(D, ...)     sancus_logger__dumpf((D),  SANCUS_LOG_DEBUG, __func__, 0, __VA_ARGS__)

#define sancus_log__notice(D, ...)         sancus_logger__printf((D), SANCUS_LOG_DEBUG, NULL, 0, __VA_ARGS__)
#define sancus_log__notice_dump(D, ...)    sancus_logger__dumpf((D),  SANCUS_LOG_DEBUG, NULL, 0, __VA_ARGS__)

#define sancus_log__if_level(D, L, F, ...) do { \
	if (sancus_logger_has_level((D), (L))) \
		sancus_log__ ##F(D, __VA_ARGS__); \
	} while(0)

#define sancus_log_error2(D, L, ...)          sancus_log__if_level((D), (L), error,  __VA_ARGS__)
#define sancus_log_error_dump2(D, L, ...)     sancus_log__if_level((D), (L), error_dump,  __VA_ARGS__)

#define sancus_log_warn2(D, L, ...)           sancus_log__if_level((D), (L), warn,   __VA_ARGS__)
#define sancus_log_warn_dump2(D, L, ...)      sancus_log__if_level((D), (L), warn_dump,   __VA_ARGS__)

#define sancus_log_info2(D, L, ...)           sancus_log__if_level((D), (L), info,   __VA_ARGS__)
#define sancus_log_info_dump2(D, L, ...)      sancus_log__if_level((D), (L), info_dump,   __VA_ARGS__)

#define sancus_log_trace2(D, L, ...)          sancus_log__if_level((D), (L), trace,  __VA_ARGS__)
#define sancus_log_trace_dump2(D, L, ...)     sancus_log__if_level((D), (L), trace_dump,  __VA_ARGS__)

#define sancus_log_debug2(D, L, ...)          sancus_log__if_level((D), (L), debug,  __VA_ARGS__)
#define sancus_log_debug_dump2(D, L, ...)     sancus_log__if_level((D), (L), debug_dump,  __VA_ARGS__)

#define sancus_log_notice2(D, L, ...)         sancus_log__if_level((D), (L), notice, __VA_ARGS__)
#define sancus_log_notice_dump2(D, L, ...)    sancus_log__if_level((D), (L), notice_dump, __VA_ARGS__)

#define sancus_log_error(...)                 sancus_log__error(__VA_ARGS__)
#define sancus_log_error_dump(...)            sancus_log__error_dump(__VA_ARGS__)

#define sancus_log_warn(D, ...)               sancus_log_warn2((D), SANCUS_LOG_WARN, __VA_ARGS__)
#define sancus_log_warn_dump(D, ...)          sancus_log_warn_dump2((D), SANCUS_LOG_WARN, __VA_ARGS__)

#define sancus_log_info(D, ...)               sancus_log_info2((D), SANCUS_LOG_INFO, __VA_ARGS__)
#define sancus_log_info_dump(D, ...)          sancus_log_info_dump2((D), SANCUS_LOG_INFO, __VA_ARGS__)

#define sancus_log_trace(D, ...)              sancus_log_trace2((D), SANCUS_LOG_TRACE, __VA_ARGS__)
#define sancus_log_trace_dump(D, ...)         sancus_log_trace_dump2((D), SANCUS_LOG_TRACE, __VA_ARGS__)

#define sancus_log_debug(D, ...)              sancus_log_debug2((D), SANCUS_LOG_DEBUG, __VA_ARGS__)
#define sancus_log_debug_dump(D, ...)         sancus_log_debug_dump2((D), SANCUS_LOG_DEBUG, __VA_ARGS__)

#define sancus_log_notice(D, ...)             sancus_log_notice2((D), SANCUS_LOG_DEBUG, __VA_ARGS__)
#define sancus_log_notice_dump(D, ...)        sancus_log_notice_dump2((D), SANCUS_LOG_DEBUG, __VA_ARGS__)

#endif /* !__SANCUS_LOGGER_H__ */
