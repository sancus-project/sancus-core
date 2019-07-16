#include <sancus/common.h>
#include <sancus/logger.h>
#include <sancus/fd.h>

#include <stdio.h>
#include <sys/uio.h>
#include <pthread.h>

#define LOG_BUFFER_SIZE 1024

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static ssize_t log_write(const char *prefix, size_t prefix_len,
			 const char *data, size_t data_len)
{
	size_t iovcnt = 0;
	struct iovec iov[3];
	ssize_t ret = 0;

	if (prefix_len > 0)
		iov[iovcnt++] = (struct iovec) { (void*)prefix, prefix_len };

	if (data_len > 0)
		iov[iovcnt++] = (struct iovec) { (void*)data, data_len };

	if (iovcnt > 0) {
		struct iovec *p = &iov[iovcnt-1];
		const char *s = p->iov_base;

		if (s[p->iov_len-1] != '\n')
			iov[iovcnt++] = (struct iovec) { "\n", 1 };

		pthread_mutex_lock(&mutex);
		ret = sancus_writev(STDERR_FILENO, iov, iovcnt);
		pthread_mutex_unlock(&mutex);
	}

	return ret;
}

static ssize_t log_fmt(char *buf, size_t buf_size,
		       const struct sancus_logger *ctx,
		       enum sancus_log_level level,
		       const char *func, unsigned line,
		       const char *fmt, va_list ap)
{
	char *p = buf;
	ssize_t l = buf_size;

	if (func && *func == '\0')
		func = NULL;
	if (fmt && *fmt == '\0')
		fmt = NULL;

	/* level */
	{
		char c = '\0';

		switch (level) {
		case SANCUS_LOG_ERR:   c = 'E'; break;
		case SANCUS_LOG_WARN:  c = 'W'; break;
		case SANCUS_LOG_INFO:  c = 'I'; break;
		case SANCUS_LOG_TRACE: c = 'T'; break;
		case SANCUS_LOG_DEBUG: c = 'D'; break;
		}

		if (c) {
			*p++ = c;
			*p++ = '/';
			l -= 2;
		}
	}

	/* logger prefix */
	if (ctx && ctx->prefix && *ctx->prefix) {
		size_t l0;

		if (fmt || func) {
			l0 = snprintf(p, l, "%s: ", ctx->prefix);
		} else {
			l0 = strlen(ctx->prefix);
			memcpy(p, ctx->prefix, l0);
		}

		l -= l0;
		p += l0;
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
	ssize_t l = log_fmt(buf, sizeof(buf),
			    ctx, level, func, line, fmt, ap);

	return log_write(buf, l, NULL, 0);
}
