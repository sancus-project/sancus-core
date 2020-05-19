#include <sancus/common.h>

#include <sancus/buffer.h>

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

ssize_t sancus_buffer_append(struct sancus_buffer *b, const char *s, ssize_t l)
{
	char *buf = sancus_buffer_tail_ptr(b);
	ssize_t buf_size = sancus_buffer_tail_size(b);

	/* s vs l */
	if (likely(s != NULL)) {
		if (l < 0)
			l = strlen(s);
	} else if (l > 0) {
		return -EINVAL;
	}

	/* buf vs l */
	if (buf == NULL) {
		return -EINVAL;
	} else if (l > buf_size) {
		return -ENOBUFS;
	} else if (l > 0) {
		memcpy(buf, s, l);
		b->len += l;
	}

	return l;
}

ssize_t sancus_buffer_appendf(struct sancus_buffer *b, const char *fmt, ...)
{
	ssize_t n, l = sancus_buffer_tail_size(b);
	char *buf = sancus_buffer_tail_ptr(b);

	if (l > 0) {
		va_list ap;
		va_start(ap, fmt);
		n = vsnprintf(buf, l, fmt, ap);
		va_end(ap);

		if (l < n)
			n = -ENOBUFS;
	} else {
		n = -ENOBUFS;
	}

	if (n > 0)
		b->len += (size_t)n;
	return n;
}
