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

enum {
	LOG_PRELUDE_SIZE =  64,
	LOG_PREFIX_SIZE  = 128,
	LOG_MESSAGE_SIZE = 256,
	LOG_DATA_SIZE    = 256,
};

/*
 * backend
 */
static const struct sancus_logger_backend fd_backend;
static const struct sancus_logger_backend *default_backend = &fd_backend;

int sancus_logger_set_default_backend(const struct sancus_logger_backend *backend)
{
	if (backend == NULL)
		backend = &fd_backend;
	else if (backend->f == NULL)
		return -EINVAL;

	default_backend = backend;
	return 0;
}

static inline
const struct sancus_logger_backend *log_find_backend(const struct sancus_logger *logger,
						     const struct sancus_logger_backend *fallback)
{
	for (;;) {
		if (logger == NULL)
			return fallback;
		else if (logger->backend && logger->backend->f)
			return logger->backend;
		else
			logger = logger->parent;
	}
}

static inline size_t log_buffer_trim(const struct sancus_buffer *buf,
				     const char **ptr)
{
	const char *base = sancus_buffer_ptr(buf);
	const char *p = sancus_buffer_tail_ptr(buf);
	size_t l;

	while (p > base) {
		char c = *--p;

		switch (c) {
		case ' ':
		case '\t':
		case '\n':
			break;
		default:
			p++;
			goto done;
		}
	}

done:
	l = (size_t)(p - base);

	if (ptr)
		*ptr = l ? base : NULL;
	return l;
}

static ssize_t log_write(const struct sancus_logger *logger,
			 unsigned level,
			 const struct sancus_buffer *pbuf,
			 const struct sancus_buffer *mbuf,
			 const struct sancus_buffer *dbuf)
{
	const char *prefix, *msg, *data;
	size_t plen, mlen, dlen;
	ssize_t rc = 0;

	plen = log_buffer_trim(pbuf, &prefix);
	mlen = log_buffer_trim(mbuf, &msg);
	dlen = log_buffer_trim(dbuf, &data);

	if (plen + mlen + dlen) {
		const struct sancus_logger_backend *d;
		d = log_find_backend(logger, default_backend);

		rc = d->f(level,
			  prefix, plen,
			  msg, mlen,
			  data, dlen,
			  d->ctx);
	}

	return rc;
}

/*
 * stderr fd backend
 */
static int fd_logger_write(unsigned level,
			   const char *prefix, size_t prefix_len,
			   const char *msg, size_t msg_len,
			   const char *data, size_t data_len,
			   void *ctx);

struct sancus_logger_fd_backend_data {
	pthread_mutex_t mutex;
	int fd;

	bool timestamp;
};

static struct sancus_logger_fd_backend_data fd_backend_data = {
	.mutex = PTHREAD_MUTEX_INITIALIZER,
	.fd = STDERR_FILENO,
	.timestamp = true,
};

static const struct sancus_logger_backend fd_backend = {
	.f = fd_logger_write,
	.ctx = &fd_backend_data,
};

int sancus_logger_set_fd_backend(int fd)
{
	if (fd < 0)
		return -EINVAL;
	fd_backend_data.fd = fd;
	return 0;
}

static inline struct iovec log_iov(const char *s, size_t l)
{
	return (struct iovec) { (void*)s, l };
}

static inline struct iovec log_iov2(const struct sancus_buffer *b)
{
	return (struct iovec) {
		sancus_buffer_ptr(b),
		sancus_buffer_len(b)
	};
}

static int fd_logger_write(unsigned level,
			   const char *prefix, size_t plen,
			   const char *msg, size_t mlen,
			   const char *data, size_t dlen,
			   void *_ctx)
{
	static const char levels[] = "EWITD";
	DECL_SANCUS_LBUFFER(prelude, LOG_PRELUDE_SIZE);
	struct sancus_logger_fd_backend_data *ctx = (struct sancus_logger_fd_backend_data *)_ctx;
	struct iovec iov[7];
	int iovcnt = 0;
	int ret = 0;

	if (ctx == NULL)
		return -EINVAL;

	/*
	 * prelude
	 */
	if (ctx->timestamp) {
		static struct timespec first, prev;
		struct timespec ts, dt0, dt1;

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
		sancus_lbuffer_appendf(&prelude,
				       "[" TIMESPEC_FMT_MS " +" TIMESPEC_FMT_MS "] ",
				       TIMESPEC_SPLIT_MS(&dt0),
				       TIMESPEC_SPLIT_MS(&dt1));
	}

	if (level < sizeof(levels))
		sancus_lbuffer_appendf(&prelude, "%c/", levels[level]);
	else
		sancus_lbuffer_appendf(&prelude, "%u/", level);

	iov[iovcnt++] = log_iov2(sancus_lbuffer_to_buffer(&prelude));

	/*
	 * prefix
	 */
	if (plen) {
		iov[iovcnt++] = log_iov(prefix, plen);

		if (mlen || dlen)
			iov[iovcnt++] = log_iov(": ", 2);
	}

	/*
	 * message
	 */
	if (mlen) {
		iov[iovcnt++] = log_iov(msg, mlen);

		if (dlen)
			iov[iovcnt++] = log_iov(": ", 2);
	}

	/*
	 * extra data
	 */
	if (dlen) {
		iov[iovcnt++] = log_iov(data, dlen);
	}

	/*
	 * write
	 */
	iov[iovcnt++] = log_iov("\n", 1);

	pthread_mutex_lock(&ctx->mutex);
	ret = (int)sancus_writev(ctx->fd, iov, iovcnt);
	pthread_mutex_unlock(&ctx->mutex);

	return ret;
}

static inline ssize_t log_ctx_prefix(const struct sancus_logger *ctx,
				     char *buf, size_t size)
{
	ssize_t rc = 0;

	if (ctx != NULL && buf != NULL && size > 0) {
		if (ctx->prefixer != NULL) {
			rc = ctx->prefixer(ctx, buf, size);
		} else if (ctx->prefix != NULL) {

			if (*ctx->prefix != '\0') {
				size_t l = strlen(ctx->prefix);

				if (size < l)
					l = size-1;
				memcpy(buf, ctx->prefix, l);
				buf[l] = '\0';

				rc = (ssize_t)l;
			}

		} else if (ctx->parent != NULL) {
			rc = log_ctx_prefix(ctx->parent, buf, size);
		}
	}

	return rc;
}

ssize_t sancus_logger_render_prefix(const struct sancus_logger *ctx, char *buf, size_t size)
{
	return log_ctx_prefix(ctx, buf, size);
}

static inline ssize_t log_ctx_prefix2(const struct sancus_logger *ctx,
				      struct sancus_buffer *b)
{
	ssize_t rc = log_ctx_prefix(ctx,
				    sancus_buffer_tail_ptr(b),
				    sancus_buffer_tail_size(b));
	if (rc > 0)
		sancus_buffer_sparse(b, (size_t)rc);

	return rc;
}

__attr_vprintf(4)
static ssize_t log_fmt(struct sancus_buffer *buf,
		       const char *func, unsigned line,
		       const char *fmt, va_list ap)
{
	ssize_t rc = 0;

	if (func && *func == '\0')
		func = NULL;
	if (fmt && *fmt == '\0')
		fmt = NULL;

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
	DECL_SANCUS_LBUFFER(pbuf0, LOG_PREFIX_SIZE);
	DECL_SANCUS_LBUFFER(mbuf0, LOG_MESSAGE_SIZE);
	struct sancus_buffer *pbuf = sancus_lbuffer_to_buffer(&pbuf0);
	struct sancus_buffer *mbuf = sancus_lbuffer_to_buffer(&mbuf0);
	ssize_t rc;

	rc = log_ctx_prefix2(ctx, pbuf);
	if (rc < 0)
		goto done;

	rc = log_fmt(mbuf, func, line, fmt, ap);
	if (rc < 0)
		goto done;

	rc = log_write(ctx, level, pbuf, mbuf, NULL);
done:
	return (int)rc;
}

int sancus_logger__printf(const struct sancus_logger *ctx,
			  enum sancus_log_level level,
			  const char *func, unsigned line,
			  const char *fmt, ...)
{
	va_list ap;
	int rc;

	va_start(ap, fmt);
	rc = sancus_logger__vprintf(ctx, level, func, line, fmt, ap);
	va_end(ap);

	return rc;
}

static inline ssize_t dump_enc(struct sancus_buffer *buf,
			       const char *sp, size_t l)
{
	static const char hexa[] = "0123456789abcdef";
	static const unsigned char CEC[] = "abtnvfr";
	const unsigned char *p = (unsigned char *)sp, *pe = p + l;
	ssize_t count = 0;

	while (p < pe) {
		unsigned char c = *p++;
		ssize_t rc = 0;

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
				char out[] = { '\\', (char)c };
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
			  const char *func, unsigned line,
			  const void * const data, size_t len,
			  const char *fmt, va_list ap)
{
	DECL_SANCUS_LBUFFER(pbuf0, LOG_PREFIX_SIZE);
	DECL_SANCUS_LBUFFER(mbuf0, LOG_MESSAGE_SIZE);
	DECL_SANCUS_LBUFFER(dbuf0, LOG_DATA_SIZE);

	struct sancus_buffer *pbuf = sancus_lbuffer_to_buffer(&pbuf0);
	struct sancus_buffer *mbuf = sancus_lbuffer_to_buffer(&mbuf0);
	struct sancus_buffer *buf = sancus_lbuffer_to_buffer(&dbuf0);

	const char *p = data;
	const char * const pe = p + len;
	const size_t min_suffix = 4 + 6; /* strlen("\" (%zu)") */
	const size_t max_suffix = min_suffix + 3; /* min_suffix + ellipsis */
	ssize_t rc;

	if (data == NULL && len > 0)
		return -EINVAL;

	rc = log_ctx_prefix2(ctx, pbuf);
	if (rc < 0)
		goto fail_rc;

	rc = log_fmt(mbuf, func, line, fmt, ap);
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

	rc = log_write(ctx, level, pbuf, mbuf, buf);
fail_rc:
	return (int)rc;
truncate:
	while ((const char *)p > (const char*)data && sancus_buffer_tail_size(buf) < max_suffix) {

		rc = dump_enc(NULL, --p, 1);
		sancus_buffer_stripn(buf, (size_t)rc);
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
			     const char *func, unsigned line,
			     size_t width, const void *data, size_t data_len,
			     const char *fmt, va_list ap)
{
	DECL_SANCUS_LBUFFER(pbuf0, LOG_PREFIX_SIZE);
	DECL_SANCUS_LBUFFER(mbuf0, LOG_MESSAGE_SIZE);
	DECL_SANCUS_LBUFFER(dbuf0, LOG_DATA_SIZE);

	struct sancus_buffer *pbuf = sancus_lbuffer_to_buffer(&pbuf0);
	struct sancus_buffer *mbuf = sancus_lbuffer_to_buffer(&mbuf0);
	struct sancus_buffer *buf = sancus_lbuffer_to_buffer(&dbuf0);

	const char *p, *pe;
	unsigned off;
	ssize_t rc;

	if (!data && data_len > 0)
		return -EINVAL;

	rc = log_ctx_prefix2(ctx, pbuf);
	if (rc < 0)
		goto fail_rc;

	rc = log_fmt(mbuf, func, line, fmt, ap);
	if (rc < 0)
		goto fail_rc;

	if (data_len == 0)
		return (int)log_write(ctx, level, pbuf, mbuf, NULL);

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
			unsigned char c = (unsigned char)*q++;
			if (c < 0x20 || c > 0x7e)
				c = '.';
			sancus_buffer_append(buf, (const char*)&c, 1);
		}

		sancus_buffer_append(buf, "|", 1);

		rc = log_write(ctx, level, pbuf, mbuf, buf);
		if (rc < 0)
			break;

		sancus_buffer_reset(buf);
	}

fail_rc:
	return (int)rc;
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
		   const char *func, unsigned line, int e,
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
		 const char *func, unsigned line, int e,
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
