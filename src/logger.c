#include <sancus/common.h>
#include <sancus/clock.h>
#include <sancus/fd.h>
#include <sancus/logger.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/uio.h>
#include <pthread.h>
#include <signal.h>

#define LOG_BUFFER_SIZE 1024
#define TS 1

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static size_t log_strcpy(char *dst, const char *src)
{
	size_t l = (src && *src) ? strlen(src) : 0;
	if (l)
		memcpy(dst, src, l + 1);
	else
		*dst = '\0';
	return l;
}

__attr_printf(3)
static ssize_t log_snprintf(char *dst, ssize_t dst_size,
			    const char *fmt, ...)
{
	ssize_t ret;
	if (dst != NULL && dst_size > 0 && fmt != NULL && *fmt != '\0') {
		va_list ap;
		va_start(ap, fmt);
		ret = vsnprintf(dst, dst_size, fmt, ap);
		va_end(ap);

		if (ret >= dst_size) {
			ret = dst_size - 1;
			dst[ret] = '\0';
		} else if (ret < 0) {
			ret = 0;
		}
	} else {
		ret = 0;
	}

	return ret;
}

static ssize_t log_write(const char *prefix, size_t prefix_len,
			 const char *data, size_t data_len)
{
	char ts_buf[32];
	size_t iovcnt = 0;
	struct iovec iov[4];
	ssize_t ret = 0;

	if (TS) {
		static struct timespec first, prev;
		struct timespec ts, dt0, dt1;
		size_t l;

		sancus_now(&ts);

		if (unlikely(sancus_time_is_zero(&prev) ||
			     sancus_time_is_lt(&ts, &first))) {
			/* first or time warped */
			dt0 = dt1 = TIMESPEC_INIT(0, 0);
			first = ts;
		} else {
			dt0 = dt1 = ts;
			sancus_time_sub(&dt0, &first);
			sancus_time_sub(&dt1, &prev);
		}

		prev = ts;
		l = snprintf(ts_buf, sizeof(ts_buf), "[" TIMESPEC_FMT_MS " +" TIMESPEC_FMT_MS "] ",
			     TIMESPEC_SPLIT_MS(&dt0),
			     TIMESPEC_SPLIT_MS(&dt1));
		iov[iovcnt++] = (struct iovec) { ts_buf, l };
	}

	if (prefix_len > 0)
		iov[iovcnt++] = (struct iovec) { (void*)prefix, prefix_len };

	if (data_len > 0)
		iov[iovcnt++] = (struct iovec) { (void*)data, data_len };

	if (iovcnt > 0) {
		struct iovec *p = &iov[iovcnt-1];
		const char *s = p->iov_base;

		if (s[p->iov_len-1] != '\n')
			iov[iovcnt++] = (struct iovec) { (char*)"\n", 1 };

		pthread_mutex_lock(&mutex);
		ret = sancus_writev(STDERR_FILENO, iov, iovcnt);
		pthread_mutex_unlock(&mutex);
	}

	return ret;
}

static inline ssize_t log_ctx_prefix(const struct sancus_logger *ctx,
				     char *buf, size_t size)
{
	size_t l = 0;

	if (ctx != NULL && buf != NULL && size > 0) {
		if (ctx->prefixer != NULL) {
			l = ctx->prefixer(ctx, buf, size);
		} else if (ctx->prefix != NULL) {

			if (*ctx->prefix != '\0') {
				l = strlen(ctx->prefix);
				if (size < l)
					l = size-1;
				memcpy(buf, ctx->prefix, l);
				buf[l] = '\0';
			}

		} else if (ctx->parent != NULL) {
			l = log_ctx_prefix(ctx->parent, buf, size);
		}
	}

	return l;
}

ssize_t sancus_logger_render_prefix(const struct sancus_logger *ctx, char *buf, size_t size)
{
	return log_ctx_prefix(ctx, buf, size);
}

__attr_vprintf(7)
static ssize_t log_fmt(char *buf, size_t buf_size,
		       const struct sancus_logger *ctx,
		       enum sancus_log_level level,
		       const char *func, unsigned line,
		       const char *fmt, va_list ap)
{
	static char levels[] = "EWITD";
	char *p = buf;
	ssize_t l = buf_size;

	if (func && *func == '\0')
		func = NULL;
	if (fmt && *fmt == '\0')
		fmt = NULL;

	/* level */
	if (level < sizeof(levels)) {
		*p++ = levels[level];
		*p++ = '/';
		l -= 2;
	}

	/* logger prefix */
	if (ctx) {
		ssize_t l0 = log_ctx_prefix(ctx, p, l);

		if (l0 > 0) {
			if (fmt || func) {
				memcpy(p + l0, ": ", 2);
				l0 += 2;
			}
			l -= l0;
			p += l0;
		}

	}

	/* source file context */
	if (func) {
		size_t l0;

		if (line) {
			l0 = snprintf(p, l, "%s:%u: ", func, line);

			if (!fmt)
				l0 -= 2;
		} else if (fmt) {
			l0 = snprintf(p, l, "%s: ", func);
		} else {
			l0 = strlen(func);
			memcpy(p, func, l0);
		}

		l -= l0;
		p += l0;
	}

	/* formatted message */
	if (fmt) {
		ssize_t l0;
		l0 = vsnprintf(p, l, fmt, ap);

		if (l0 < 0) {
			const char *s = "<invalid format>";
			l0 = strlen(s);
			memcpy(p, s, l0 + 1);
		} else if (l0 >= l) {
			/* truncated */
			l0 = l - 1;
		}

		p += l0;
		l -= l0;
	}

	if (p != buf) {
		if (l < 1) {
			p = buf + buf_size;
			p -= 2;
		}

		if (p[-1] == '\n')
			p--;

		*p = '\0';
	}

	return p - buf;
}

int sancus_logger__vprintf(const struct sancus_logger *ctx,
			   enum sancus_log_level level,
			   const char *func, unsigned line,
			   const char *fmt, va_list ap)
{
	char buf[LOG_BUFFER_SIZE];
	ssize_t l;

	l = log_fmt(buf, sizeof(buf), ctx, level, func, line, fmt, ap);

	return log_write(buf, l, NULL, 0);
}

int sancus_logger__printf(const struct sancus_logger *ctx,
			  enum sancus_log_level level,
			  const char *func, unsigned line,
			  const char *fmt, ...)
{
	char buf[LOG_BUFFER_SIZE];
	ssize_t l;
	va_list ap;
	int err;

	va_start(ap, fmt);
	l = log_fmt(buf, sizeof(buf), ctx, level, func, line, fmt, ap);
	err = log_write(buf, l, NULL, 0);
	va_end(ap);

	return err;
}

int sancus_logger__vdumpf(const struct sancus_logger *ctx,
			  enum sancus_log_level level,
			  const char *func, size_t line,
			  const void *data, size_t data_len,
			  const char *fmt, va_list ap)
{
	static const char hexa[] = "0123456789abcdef";
	static const char CEC[] = "abtnvfr";
	char buf[1024];
	const char *p = data, *pe = p + data_len;
	char *out = buf, *oute = buf + sizeof(buf);
	const size_t max_suffix = 17; /* strlen("... (%zu)\n") */
	const char *outs = oute - max_suffix; /* maximum location before the suffix */
	ssize_t l;

	l = log_fmt(buf, sizeof(buf),
		    ctx, level, func, line, fmt, ap);
	if (l < 0)
		return l;
	out += l;

	if (unlikely(data == NULL)) {
		out += log_strcpy(out, "nil\n");
		goto write_buf;
	}

	*out++ = '"';
	while (p < pe && out < outs) {
		char c = *p++;
		if (c > 0x1f && c < 0x7f) { /* ASCII printable characters */
			switch(c) {
				case '"':
				case '\\':
					goto escape2;
				default:
					*out++ = c;
			}
		} else if (c >= '\a' && c <= '\r') {
			/* C Character Escape Codes */
			c = CEC[c - '\a'];
escape2:
			*out++ = '\\';
			*out++ = c;
		} else if (c == 0) {
			c = '0';
			goto escape2;
		} else { /* not printable, hexa encoded */
			*out++ = '\\';
			*out++ = 'x';
			*out++ = hexa[(c & (0x0f << 4)) >> 4];
			*out++ = hexa[c & 0x0f];
		}
	}

	out += log_snprintf(out, oute - out,
			    (p < pe) ?  "... (%zu)\n" : "\" (%zu)\n",
			    data_len);

write_buf:
	return log_write(buf, out - buf, NULL, 0);
}

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

int sancus_logger__vhexdumpf(const struct sancus_logger *ctx,
			     enum sancus_log_level level,
			     const char *func, size_t line,
			     size_t width, const void *data, size_t data_len,
			     const char *fmt, va_list ap)
{
	char buf[1024];
	const char *p = data, *pe = p + data_len;
	ssize_t base = log_fmt(buf, sizeof(buf),
			       ctx, level, func, line, fmt, ap);
	unsigned off = 0;
	int rc = 0;

	if (base < 0)
		rc = base;

	while (rc >= 0 && p < pe) {
		const char *q = p;
		size_t l = base;

		if (width) {
			unsigned i;
			l += snprintf(buf + l, sizeof(buf) - l, "%08x ", off);
			for (i = 0; i < width && p < pe; i++, off++)
				l += snprintf(buf + l, sizeof(buf) - l, " %02x", *p++ & 0xff);

			/* align */
			for (; i < width; l += 3, i++)
				memcpy(buf + l, "   ", 3);
		} else {
			for (; p < pe; off++)
				l += snprintf(buf + l, sizeof(buf) - l, " %02x", *p++ & 0xff);
		}

		/* ascii */
		memcpy(buf + l, " |", 2); l += 2;
		while (q < p) {
			unsigned char c = *q++;
			buf[l++] = (c < 0x20 || c > 0x7e) ? '.' : c;
		}
		memcpy(buf + l, "|\n", 3); l += 2;

		rc = log_write(buf, l, NULL, 0);
	}

	return rc;
}

int sancus_logger__hexdumpf(const struct sancus_logger *log,
			    enum sancus_log_level level,
			    const char *func, unsigned line,
			    size_t width, const void *data, size_t data_len,
			    const char *fmt, ...)
{
	va_list ap;
	int err;
	va_start(ap, fmt);
	err = sancus_logger__vhexdumpf(log, level, func, line,
				       width, data, data_len,
				       fmt, ap);
	va_end(ap);
	return err;
}

/*
 * assert()
 */
int sancus__assert(const struct sancus_logger *log, int ndebug,
		   const char *func, size_t line, int e,
		   const char *fmt, ...)
{
	if (unlikely(!e)) {
		va_list ap;
		va_start(ap, fmt);
		sancus_logger__vprintf(log, SANCUS_LOG_ERROR_BIT, func, line, fmt, ap);
		va_end(ap);

		if (!ndebug)
			abort();
	}
	return e;
}

/*
 * trap()
 */
int sancus__trap(const struct sancus_logger *log, int ndebug,
		 const char *func, size_t line, int e,
		 const char *fmt, ...)
{
	if (e) {
		va_list ap;
		va_start(ap, fmt);
		sancus_logger__vprintf(log, SANCUS_LOG_WARN_BIT, func, line, fmt, ap);
		va_end(ap);

		if (!ndebug && ptrace(PTRACE_TRACEME, 0, NULL, 0) == -1)
			raise(SIGTRAP);
	}
	return e;
}
