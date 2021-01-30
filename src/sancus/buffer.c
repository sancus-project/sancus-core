#include <sancus/common.h>

#include <sancus/buffer.h>

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

/*
 * strip
 */
ssize_t sancus_buffer__stripchar(struct sancus_buffer *b, bool many,
				 const char *chars, ssize_t l)
{
	size_t count = 0;

	if (unlikely(b == NULL))
		return -EINVAL;
	else if (unlikely(chars == NULL && l > 0))
		return -EINVAL;

	if (l < 0)
		l = chars ? (ssize_t)strlen(chars) : 0;

	if (l > 0) {
		const char *p, *pe, *q, *qe;

		p = sancus_buffer_tail_ptr(b) - 1;
		pe = sancus_buffer_ptr(b) - 1;
		qe = chars + l;

		while (p > pe) {

			for (q = chars; q < qe; q++) {
				if (*p == *q)
					goto match;
			}

			break;
match:
			count++;
			p--;

			if (!many)
				break;
		}
	}

	if (count)
		b->len -= count;

	return (ssize_t)count;
}

ssize_t sancus_buffer_strip(struct sancus_buffer *b, const char *s, ssize_t l)
{
	size_t bl = sancus_buffer_len(b);

	/* s vs l */
	if (likely(s != NULL)) {
		if (l < 0)
			l = (ssize_t)strlen(s);
	} else if (l > 0) {
		return -EINVAL;
	}

	if (l > 0 && bl >= (size_t)l) {
		size_t lu = (size_t)l;
		size_t off = bl - lu;
		char *p = sancus_buffer_ptr(b) + off;

		if (memcmp(p, s, lu) == 0) {
			/* match, remove */
			b->len -= lu;
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

	return (ssize_t)l;
}

/*
 * append
 */
ssize_t sancus_buffer__append(struct sancus_buffer *b, bool truncate,
			      const char *s, ssize_t l)
{
	char *buf = sancus_buffer_tail_ptr(b);
	ssize_t buf_size = (ssize_t)sancus_buffer_tail_size(b);

	/* s vs l */
	if (likely(s != NULL)) {
		if (l < 0)
			l = (ssize_t)strlen(s);
	} else if (l > 0) {
		return -EINVAL;
	}

	/* buf vs l */
	if (buf == NULL)
		l = -EINVAL;
	else if (l <= buf_size)
		;
	else if (truncate)
		l = buf_size;
	else
		l = -ENOBUFS;

	if (l > 0) {
		memcpy(buf, s, (size_t)l);
		b->len += (size_t)l;
	}

	return l;
}

ssize_t sancus_buffer__appendv(struct sancus_buffer *b, bool truncate,
			       const char *fmt, va_list ap)
{
	ssize_t n, l = (ssize_t)sancus_buffer_tail_size(b);
	char *buf = sancus_buffer_tail_ptr(b);

	if (l > 0) {
		n = vsnprintf(buf, (size_t)l, fmt, ap);

		if (n <= 0 || l > n) {
			;
		} else if (truncate) {
			n = l - 1;
			buf[n] = '\0';
		} else {
			n = -ENOBUFS;
		}
	} else {
		n = -ENOBUFS;
	}

	if (n > 0)
		b->len += (size_t)n;
	return n;
}

ssize_t sancus_buffer__appendf(struct sancus_buffer *b, bool truncate,
			       const char *fmt, ...)
{
	ssize_t rc;
	va_list ap;

	va_start(ap, fmt);
	rc = sancus_buffer__appendv(b, truncate, fmt, ap);
	va_end(ap);

	return rc;
}
