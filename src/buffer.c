#include <sancus/common.h>

#include <sancus/buffer.h>

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

ssize_t sancus_buffer_strip(struct sancus_buffer *b, const char *s, ssize_t l)
{
	ssize_t bl = sancus_buffer_len(b);

	/* s vs l */
	if (likely(s != NULL)) {
		if (l < 0)
			l = strlen(s);
	} else if (l > 0) {
		return -EINVAL;
	}

	if (l > 0 && bl >= l) {
		size_t off = bl - l;
		char *p = sancus_buffer_ptr(b) + off;

		if (memcmp(p, s, l) == 0) {
			/* match, remove */
			b->len -= l;
			*p = '\0';
			return l;
		}
	}

	return 0;
}

ssize_t sancus_buffer_stripn(struct sancus_buffer *b, size_t n)
{
	size_t l = sancus_buffer_len(b);

	if (n < l) {
		l -= n;
		b->buf[l - 1] = '\0';
	} else {
		b->base = b->len = 0;
		b->buf[0] = '\0';
	}

	return l;
}

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

ssize_t sancus_buffer_appendv(struct sancus_buffer *b, const char *fmt, va_list ap)
{
	ssize_t n, l = sancus_buffer_tail_size(b);
	char *buf = sancus_buffer_tail_ptr(b);

	if (l > 0) {
		n = vsnprintf(buf, l, fmt, ap);

		if (l < n)
			n = -ENOBUFS;
	} else {
		n = -ENOBUFS;
	}

	if (n > 0)
		b->len += (size_t)n;
	return n;
}

ssize_t sancus_buffer_appendf(struct sancus_buffer *b, const char *fmt, ...)
{
	ssize_t rc;
	va_list ap;

	va_start(ap, fmt);
	rc = sancus_buffer_appendv(b, fmt, ap);
	va_end(ap);

	return rc;
}
