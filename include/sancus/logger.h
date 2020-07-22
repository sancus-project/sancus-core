#ifndef __SANCUS_LOGGER_H__
#define __SANCUS_LOGGER_H__

#include <sancus/bit.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

enum sancus_log_level {
	SANCUS_LOG_ERROR_BIT,
	SANCUS_LOG_WARN_BIT,
	SANCUS_LOG_INFO_BIT,
	SANCUS_LOG_TRACE_BIT,
	SANCUS_LOG_DEBUG_BIT,

	SANCUS_LOG_NEXT_BIT = 8,
};

enum {
	SANCUS_LOG_ERR = BIT(SANCUS_LOG_ERROR_BIT),
	SANCUS_LOG_WARN = BIT(SANCUS_LOG_WARN_BIT),
	SANCUS_LOG_INFO = BIT(SANCUS_LOG_INFO_BIT),
	SANCUS_LOG_TRACE = BIT(SANCUS_LOG_TRACE_BIT),
	SANCUS_LOG_DEBUG = BIT(SANCUS_LOG_DEBUG_BIT),

	SANCUS_LOG_NEXT  = BIT(SANCUS_LOG_NEXT_BIT),

	SANCUS_LOG_QUIET   = SANCUS_LOG_ERR | SANCUS_LOG_WARN,
	SANCUS_LOG_NORMAL  = SANCUS_LOG_QUIET | SANCUS_LOG_INFO,
	SANCUS_LOG_VERBOSE = SANCUS_LOG_NORMAL | SANCUS_LOG_DEBUG,
};

struct sancus_logger {
	const struct sancus_logger *parent;

	const char *prefix;
	unsigned mask;
	ssize_t (*prefixer) (const struct sancus_logger *, char *, size_t);
};

#define SANCUS__LOGGER_INIT(S, M) { \
	.prefix = (S), \
	.mask = (M) == 0 ? SANCUS_LOG_NORMAL : (M), \
}

#define SANCUS_LOGGER_INIT(S, M) (struct sancus_logger) SANCUS__LOGGER_INIT(S, M)
#define DECL_SANCUS_LOGGER(N, S, M) struct sancus_logger N = SANCUS__LOGGER_INIT(S, M)

static inline void sancus_logger_init2(struct sancus_logger *log,
				       const struct sancus_logger *parent,
				       ssize_t (*f) (const struct sancus_logger *, char *, size_t),
				       unsigned mask,
				       const char *prefix)
{
	if (log != NULL) {
		if (mask == 0 && parent == NULL)
			mask = SANCUS_LOG_NORMAL;

		*log = (struct sancus_logger) {
			.parent = parent,

			.prefix = prefix,
			.mask = mask,
			.prefixer = f,
		};
	}
}

static inline
void sancus_logger_init(struct sancus_logger *log,
		       const char *prefix, unsigned mask)
{
	sancus_logger_init2(log, NULL, NULL, mask, prefix);
}

static inline
const struct sancus_logger *sancus_logger_get_parent(const struct sancus_logger *log)
{
	return log != NULL ? log->parent : NULL;
}

static inline
void sancus_logger_set_prefix(struct sancus_logger *log,
			      const char *prefix)
{
	log->prefix = prefix;
}

static inline
const char *sancus_logger_get_prefix(const struct sancus_logger *log)
{
	return log != NULL ? log->prefix : NULL;
}

static inline
void sancus_logger_set_prefixer(struct sancus_logger *log,
				ssize_t (*f) (const struct sancus_logger *, char *, size_t))
{
	log->prefixer = f;
}

ssize_t sancus_logger_render_prefix(const struct sancus_logger *, char *, size_t);

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
	log->mask = (unsigned)sancus_bit_cms(log->mask, mask, mask, 0);
}

static inline
void sancus_logger_unset_mask(struct sancus_logger *log,
			      unsigned mask)
{
	log->mask = (unsigned)sancus_bit_cms(log->mask, 0, mask, 0);
}

static inline
int sancus_logger_has_level(const struct sancus_logger *log,
			    unsigned mask)
{
	if (!mask || mask&SANCUS_LOG_ERR)
		mask = 1;
	else if (log == NULL)
		mask &= SANCUS_LOG_NORMAL;
	else if (log->mask != 0)
		mask &= log->mask;
	else if (log->parent == NULL)
		mask &= SANCUS_LOG_NORMAL;
	else
		return sancus_logger_has_level(log->parent, mask);

	return !!mask;
}

/*
 */
__attr_vprintf(5)
int sancus_logger__vprintf(const struct sancus_logger *log,
			   enum sancus_log_level level,
			   const char *func, unsigned line,
			   const char *fmt, va_list ap);

__attr_printf(5)
int sancus_logger__printf(const struct sancus_logger *log,
			  enum sancus_log_level level,
			  const char *func, unsigned line,
			  const char *fmt, ...);

/*
 * logs data in printf string format
 */
__attr_vprintf(7)
int sancus_logger__vdumpf(const struct sancus_logger *log,
			  enum sancus_log_level level,
			  const char *func, size_t line,
			  const void *data, size_t data_len,
			  const char *fmt, va_list ap);

__attr_printf(7)
int sancus_logger__dumpf(const struct sancus_logger *log,
			  enum sancus_log_level level,
			  const char *func, unsigned line,
			  const void *data, size_t data_len,
			  const char *fmt, ...);

/*
 * logs data in `hexdump -C` format
 */
__attr_vprintf(8)
int sancus_logger__vhexdumpf(const struct sancus_logger *log,
			     enum sancus_log_level level,
			     const char *func, size_t line,
			     size_t width, const void *data, size_t data_len,
			     const char *fmt, va_list ap);

__attr_printf(8)
int sancus_logger__hexdumpf(const struct sancus_logger *log,
			    enum sancus_log_level level,
			    const char *func, unsigned line,
			    size_t width, const void *data, size_t data_len,
			    const char *fmt, ...);

/*
 * assert()
 */
__attr_printf(6)
int sancus__assert(const struct sancus_logger *log, int ndebug,
		   const char *func, size_t line, int e,
		   const char *fmt, ...);

#ifdef NDEBUG
#define sancus__assert2(L, ...) likely(sancus__assert((L), 1, __VA_ARGS__))
#else
#define sancus__assert2(L, ...) likely(sancus__assert((L), 0, __VA_ARGS__))
#endif

#define sancus_assertf(L, E, ...) sancus__assert2((L), __func__, __LINE__, (E), "assertion failed: " __VA_ARGS__)
#define sancus_assert(L, E)       sancus__assert2((L), __func__, __LINE__, (E), "assertion `%s` failed.", #E)

/*
 */
#define sancus_log__error(D, ...)          sancus_logger__printf((D),   SANCUS_LOG_ERROR_BIT,   NULL, 0, __VA_ARGS__)
#define sancus_log__error_dump(D, ...)     sancus_logger__dumpf((D),    SANCUS_LOG_ERROR_BIT,   NULL, 0, __VA_ARGS__)
#define sancus_log__error_hexdump(D, ...)  sancus_logger__hexdumpf((D), SANCUS_LOG_ERROR_BIT,   NULL, 0, __VA_ARGS__)

#define sancus_log__warn(D, ...)           sancus_logger__printf((D),   SANCUS_LOG_WARN_BIT,  NULL, 0, __VA_ARGS__)
#define sancus_log__warn_dump(D, ...)      sancus_logger__dumpf((D),    SANCUS_LOG_WARN_BIT,  NULL, 0, __VA_ARGS__)
#define sancus_log__warn_hexdump(D, ...)   sancus_logger__hexdumpf((D), SANCUS_LOG_WARN_BIT,  NULL, 0, __VA_ARGS__)

#define sancus_log__info(D, ...)           sancus_logger__printf((D),   SANCUS_LOG_INFO_BIT,  NULL, 0, __VA_ARGS__)
#define sancus_log__info_dump(D, ...)      sancus_logger__dumpf((D),    SANCUS_LOG_INFO_BIT,  NULL, 0, __VA_ARGS__)
#define sancus_log__info_hexdump(D, ...)   sancus_logger__hexdumpf((D), SANCUS_LOG_INFO_BIT,  NULL, 0, __VA_ARGS__)

#define sancus_log__trace(D, ...)          sancus_logger__printf((D),   SANCUS_LOG_TRACE_BIT, __func__, __LINE__, __VA_ARGS__)
#define sancus_log__trace_dump(D, ...)     sancus_logger__dumpf((D),    SANCUS_LOG_TRACE_BIT, __func__, __LINE__, __VA_ARGS__)
#define sancus_log__trace_hexdump(D, ...)  sancus_logger__hexdumpf((D), SANCUS_LOG_TRACE_BIT, __func__, __LINE__, __VA_ARGS__)

#define sancus_log__debug(D, ...)          sancus_logger__printf((D),   SANCUS_LOG_DEBUG_BIT, __func__, 0, __VA_ARGS__)
#define sancus_log__debug_dump(D, ...)     sancus_logger__dumpf((D),    SANCUS_LOG_DEBUG_BIT, __func__, 0, __VA_ARGS__)
#define sancus_log__debug_hexdump(D, ...)  sancus_logger__hexdumpf((D), SANCUS_LOG_DEBUG_BIT, __func__, 0, __VA_ARGS__)

#define sancus_log__notice(D, ...)         sancus_logger__printf((D),   SANCUS_LOG_DEBUG_BIT, NULL, 0, __VA_ARGS__)
#define sancus_log__notice_dump(D, ...)    sancus_logger__dumpf((D),    SANCUS_LOG_DEBUG_BIT, NULL, 0, __VA_ARGS__)
#define sancus_log__notice_hexdump(D, ...) sancus_logger__hexdumpf((D), SANCUS_LOG_DEBUG_BIT, NULL, 0, __VA_ARGS__)

#define sancus_log__if_level(D, L, F, ...) do { \
	if (sancus_logger_has_level((D), (L))) \
		sancus_log__ ##F(D, __VA_ARGS__); \
	} while(0)

#define sancus_log_error2(D, L, ...)          sancus_log__if_level((D), (L), error,  __VA_ARGS__)
#define sancus_log_error_dump2(D, L, ...)     sancus_log__if_level((D), (L), error_dump,  __VA_ARGS__)
#define sancus_log_error_hexdump2(D, L, ...)  sancus_log__if_level((D), (L), error_hexdump,  __VA_ARGS__)

#define sancus_log_warn2(D, L, ...)           sancus_log__if_level((D), (L), warn,   __VA_ARGS__)
#define sancus_log_warn_dump2(D, L, ...)      sancus_log__if_level((D), (L), warn_dump,   __VA_ARGS__)
#define sancus_log_warn_hexdump2(D, L, ...)   sancus_log__if_level((D), (L), warn_hexdump,   __VA_ARGS__)

#define sancus_log_info2(D, L, ...)           sancus_log__if_level((D), (L), info,   __VA_ARGS__)
#define sancus_log_info_dump2(D, L, ...)      sancus_log__if_level((D), (L), info_dump,   __VA_ARGS__)
#define sancus_log_info_hexdump2(D, L, ...)   sancus_log__if_level((D), (L), info_hexdump,   __VA_ARGS__)

#define sancus_log_trace2(D, L, ...)          sancus_log__if_level((D), (L), trace,  __VA_ARGS__)
#define sancus_log_trace_dump2(D, L, ...)     sancus_log__if_level((D), (L), trace_dump,  __VA_ARGS__)
#define sancus_log_trace_hexdump2(D, L, ...)  sancus_log__if_level((D), (L), trace_hexdump,  __VA_ARGS__)

#define sancus_log_debug2(D, L, ...)          sancus_log__if_level((D), (L), debug,  __VA_ARGS__)
#define sancus_log_debug_dump2(D, L, ...)     sancus_log__if_level((D), (L), debug_dump,  __VA_ARGS__)
#define sancus_log_debug_hexdump2(D, L, ...)  sancus_log__if_level((D), (L), debug_hexdump,  __VA_ARGS__)

#define sancus_log_notice2(D, L, ...)         sancus_log__if_level((D), (L), notice, __VA_ARGS__)
#define sancus_log_notice_dump2(D, L, ...)    sancus_log__if_level((D), (L), notice_dump, __VA_ARGS__)
#define sancus_log_notice_hexdump2(D, L, ...) sancus_log__if_level((D), (L), notice_hexdump, __VA_ARGS__)

#define sancus_log_error(...)                 sancus_log__error(__VA_ARGS__)
#define sancus_log_error_dump(...)            sancus_log__error_dump(__VA_ARGS__)
#define sancus_log_error_hexdump(...)         sancus_log__error_hexdump(__VA_ARGS__)

#define sancus_log_warn(D, ...)               sancus_log_warn2((D), SANCUS_LOG_WARN, __VA_ARGS__)
#define sancus_log_warn_dump(D, ...)          sancus_log_warn_dump2((D), SANCUS_LOG_WARN, __VA_ARGS__)
#define sancus_log_warn_hexdump(D, ...)       sancus_log_warn_hexdump2((D), SANCUS_LOG_WARN, __VA_ARGS__)

#define sancus_log_info(D, ...)               sancus_log_info2((D), SANCUS_LOG_INFO, __VA_ARGS__)
#define sancus_log_info_dump(D, ...)          sancus_log_info_dump2((D), SANCUS_LOG_INFO, __VA_ARGS__)
#define sancus_log_info_hexdump(D, ...)       sancus_log_info_hexdump2((D), SANCUS_LOG_INFO, __VA_ARGS__)

#define sancus_log_trace(D, ...)              sancus_log_trace2((D), SANCUS_LOG_TRACE, __VA_ARGS__)
#define sancus_log_trace_dump(D, ...)         sancus_log_trace_dump2((D), SANCUS_LOG_TRACE, __VA_ARGS__)
#define sancus_log_trace_hexdump(D, ...)      sancus_log_trace_hexdump2((D), SANCUS_LOG_TRACE, __VA_ARGS__)

#define sancus_log_debug(D, ...)              sancus_log_debug2((D), SANCUS_LOG_DEBUG, __VA_ARGS__)
#define sancus_log_debug_dump(D, ...)         sancus_log_debug_dump2((D), SANCUS_LOG_DEBUG, __VA_ARGS__)
#define sancus_log_debug_hexdump(D, ...)      sancus_log_debug_hexdump2((D), SANCUS_LOG_DEBUG, __VA_ARGS__)

#define sancus_log_notice(D, ...)             sancus_log_notice2((D), SANCUS_LOG_DEBUG, __VA_ARGS__)
#define sancus_log_notice_dump(D, ...)        sancus_log_notice_dump2((D), SANCUS_LOG_DEBUG, __VA_ARGS__)
#define sancus_log_notice_hexdump(D, ...)     sancus_log_notice_hexdump2((D), SANCUS_LOG_DEBUG, __VA_ARGS__)

/*
 */
#define sancus_log_perror2(D, E, F, ...)      sancus_log_error((D), F ": %s (%d)", __VA_ARGS__, strerror(E), (E))
#define sancus_log_perror(D, F, ...)          sancus_log_perror2((D), errno, F, __VA_ARGS__)

#endif /* !__SANCUS_LOGGER_H__ */
