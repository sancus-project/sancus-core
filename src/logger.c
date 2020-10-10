#include <sancus/common.h>
#include <sancus/clock.h>
#include <sancus/fd.h>
#include <sancus/logger.h>
#include <sancus/buffer_local.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/uio.h>
#include <pthread.h>
#include <signal.h>

#define LOG_BUFFER_SIZE 1024
#define TS 1

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int log_write(const struct sancus_buffer *buf)
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

	if (sancus_buffer_len(buf) > 0)
		iov[iovcnt++] = (struct iovec) {
			sancus_buffer_ptr(buf),
			sancus_buffer_len(buf)
		};

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

__attr_vprintf(6)
static ssize_t log_fmt(const struct sancus_logger *ctx,
		       struct sancus_buffer *buf,
		       enum sancus_log_level level,
		       const char *func, unsigned line,
		       const char *fmt, va_list ap)
{
	int rc = 0;
	static char levels[] = "EWITD";

	if (func && *func == '\0')
		func = NULL;
	if (fmt && *fmt == '\0')
		fmt = NULL;

	/* level */
	if (level < sizeof(levels)) {
		rc = sancus_buffer_appendf(buf, "%c/", levels[level]);
		if (rc < 0)
			goto fail_rc;
	}

	/* logger prefix */
	if (ctx) {
		rc = log_ctx_prefix(ctx, sancus_buffer_tail_ptr(buf),
				    sancus_buffer_tail_size(buf));
		if (rc > 0) {
			sancus_buffer_sparse(buf, rc);
			sancus_buffer_append(buf, ": ", 2);
		} else if (rc < 0) {
			goto fail_rc;
		}

	}

	/* source file context */
	if (func) {
		sancus_buffer_appendz(buf, func);

		if (line)
			sancus_buffer_appendf(buf, ":%u", line);

		sancus_buffer_append(buf, ": ", 2);
	}

	/* formatted message */
	if (fmt) {
		rc = sancus_buffer__appendv(buf, true, fmt, ap);
		if (rc < 0)
			goto fail_rc;
	}

	sancus_buffer_stripany(buf, "\n\t :");
fail_rc:
	return rc;
}

int sancus_logger__vprintf(const struct sancus_logger *ctx,
			   enum sancus_log_level level,
			   const char *func, unsigned line,
			   const char *fmt, va_list ap)
{
	DECL_SANCUS_LBUFFER(lbuf, LOG_BUFFER_SIZE);
	struct sancus_buffer *buf = sancus_lbuffer_to_buffer(&lbuf);
	int rc;

	rc = log_fmt(ctx, buf, level, func, line, fmt, ap);
	if (rc > 0)
		return log_write(buf);
	return rc;
}

int sancus_logger__printf(const struct sancus_logger *ctx,
			  enum sancus_log_level level,
			  const char *func, unsigned line,
			  const char *fmt, ...)
{
	DECL_SANCUS_LBUFFER(lbuf, LOG_BUFFER_SIZE);
	struct sancus_buffer *buf = sancus_lbuffer_to_buffer(&lbuf);
	va_list ap;
	int rc;

	va_start(ap, fmt);
	rc = log_fmt(ctx, buf, level, func, line, fmt, ap);
	if (rc > 0)
		rc = log_write(buf);
	va_end(ap);

	return rc;
}

static inline size_t dump_enc(struct sancus_buffer *buf,
			      const char *p, size_t l)
{
	static const char hexa[] = "0123456789abcdef";
	static const char CEC[] = "abtnvfr";
	const char *pe = p + l;
	size_t count;

	while (p < pe) {
		unsigned char c = *p++;
		int rc = 0;

		if (c > 0x1f && c < 0x7f) {
			/* ASCII printable characters */
			switch (c) {
			case '"':
			case '\\':
				goto escape2;
			default:
				count += 1;
				if (buf)
					rc = sancus_buffer_append(buf,
								  (const char *)&c,
								  1);
			}
		} else if (c >= '\a' && c <= '\r') {
			/* C Character Escape Codes */
			c = CEC[c - '\a'];
escape2:
			count += 2;
			if (buf) {
				char out[] = { '\\', c };
				rc =sancus_buffer_append(buf, out, 2);
			}
		} else { /* not printable, hexa encoded */
			count += 4;

			if (buf) {
				char out[] = {
					'\\', 'x',
					hexa[(c & (0x0f << 4)) >> 4],
					hexa[c & 0x0f]};

				rc =sancus_buffer_append(buf, out, 2);
			}
		}

		if (rc < 0)
			return rc;
	}

	return count;
}


int sancus_logger__vdumpf(const struct sancus_logger *ctx,
			  enum sancus_log_level level,
			  const char *func, size_t line,
			  const void * const data, size_t len,
			  const char *fmt, va_list ap)
{
	DECL_SANCUS_LBUFFER(lbuf, LOG_BUFFER_SIZE);
	struct sancus_buffer *buf = sancus_lbuffer_to_buffer(&lbuf);
	const char *p = data;
	const char * const pe = p + len;
	const size_t min_suffix = 4 + 6; /* strlen("\" (%zu)") */
	const size_t max_suffix = min_suffix + 3; /* min_suffix + ellipsis */
	int rc;

	if (data == NULL && len > 0)
		return -EINVAL;

	rc = log_fmt(ctx, buf, level, func, line, fmt, ap);
	if (rc < 0)
		goto fail_rc;

	rc = sancus_buffer_append(buf, "\"", 1);
	if (rc < 0)
		goto fail_rc;

	while (p < pe) {
		size_t bytes;

		/* to be safe, encode blocks that would fit in the worst case */
		bytes = sancus_buffer_tail_size(buf) - min_suffix;
		bytes = bytes / 4;
		bytes = len > bytes ? bytes : len;

		if (unlikely(bytes == 0))
			goto truncate;

		dump_enc(buf, p, bytes);
		p += bytes;
		len -= bytes;
	}

done:
	/* append min suffix */
	rc = sancus_buffer_appendf(buf, "\" (%zu)", (const char*)pe - (const char*)data);
	if (unlikely(rc < 0))
		goto fail_rc;

	rc = log_write(buf);
fail_rc:
	return rc;
truncate:
	while ((const char *)p > (const char*)data && sancus_buffer_tail_size(buf) < max_suffix) {

		rc = dump_enc(NULL, --p, 1);
		sancus_buffer_stripn(buf, rc);
	}

	/* append max suffix */
	rc = sancus_buffer_append(buf, "...", 3);
	if (unlikely(rc < 0))
		goto fail_rc;
	goto done;
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
	DECL_SANCUS_LBUFFER(lbuf, LOG_BUFFER_SIZE);
	struct sancus_buffer *buf = sancus_lbuffer_to_buffer(&lbuf);
	const char *p, *pe;
	unsigned base, off;
	int rc;

	if (!data && data_len > 0)
		return -EINVAL;

	rc = log_fmt(ctx, buf, level, func, line, fmt, ap);
	if (rc > 0 && data_len > 0)
		rc = sancus_buffer_append(buf, ": ", 2);
	if (rc < 0)
		return rc;
	else if (data_len == 0)
		return log_write(buf);

	base = sancus_buffer_len(buf);
	off = 0;
	p = data;
	pe = p + data_len;

	while (p < pe) {
		const char *q = p;

		if (width) {
			size_t i;
			sancus_buffer_appendf(buf, "%08x ", off);
			for (i = 0; i < width && p < pe; i++, off++)
				sancus_buffer_appendf(buf, " %02x", *p++ & 0xff);

			/* align */
			for (; i < width; i++)
				sancus_buffer_append(buf, "   ", 3);
		} else {
			for (; p < pe; off++)
				sancus_buffer_appendf(buf, " %02x", *p++ & 0xff);
		}

		/* ascii */
		sancus_buffer_append(buf, " |", 2);
		while (q < p) {
			unsigned char c = *q++;
			if (c < 0x20 || c > 0x7e)
				c = '.';
			sancus_buffer_append(buf, (const char*)&c, 1);
		}

		sancus_buffer_append(buf, "|", 1);

		rc = log_write(buf);
		if (rc < 0)
			break;

		sancus_buffer_truncate(buf, base);
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
