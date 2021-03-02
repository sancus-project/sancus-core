#ifndef __SANCUS_BUFFER_H__
#define __SANCUS_BUFFER_H__

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

struct sancus_buffer {
	char *buf;
	uint_fast16_t base, len, size;
};

static inline int sancus_buffer_init(struct sancus_buffer *b,
				     char *buf, unsigned size)
{
	if (b == NULL || buf == NULL || size == 0)
		return -EINVAL;

	*b = (struct sancus_buffer) {
		.buf = buf,
		.size = size,
	};

	return 0;
}

/*
 * accessors
 */
#define sancus_buffer_base(B) ((B) != NULL ? (B)->base : 0)
#define sancus_buffer_len(B)  ((B) != NULL ? (B)->len  : 0)
#define sancus_buffer_size(B) ((B) != NULL ? (B)->size : 0)
#define sancus_buffer_ptr(B)  ((B) != NULL ? (B)->buf + (B)->base : NULL)

#define sancus_buffer_tail_size(B) ((B) != NULL ? (B)->size - (B)->base - (B)->len : 0)
#define sancus_buffer_tail_ptr(B)  ((B) != NULL ? (B)->buf  + (B)->base + (B)->len : NULL)

#define sancus_buffer_is_empty(B) (sancus_buffer_len(B)  == 0)
#define sancus_buffer_is_off(B)   (sancus_buffer_base(B) != 0)

/*
 * strip
 */
ssize_t sancus_buffer__stripchar(struct sancus_buffer *, bool, const char *, ssize_t);

ssize_t sancus_buffer_strip(struct sancus_buffer *, const char *s, ssize_t);

static inline ssize_t sancus_buffer_stripz(struct sancus_buffer *b, const char *s)
{
	if (s) {
		ssize_t l = (ssize_t)strlen(s);
		return sancus_buffer_strip(b, s, l);
	}
	return 0;
}

ssize_t sancus_buffer_truncate(struct sancus_buffer *, size_t);
ssize_t sancus_buffer_stripn(struct sancus_buffer *, size_t);

#define sancus_buffer_striponce(B, S) sancus_buffer__stripchar((B), false, (S), -1)
#define sancus_buffer_stripany(B, S)  sancus_buffer__stripchar((B), true, (S), -1)

#define sancus_buffer_pop(B, N) sancus_buffer_stripn((B), (N))
#define sancus_buffer_reset(B)  sancus_buffer_truncate((B), 0)

/*
 * append
 */
static inline ssize_t sancus_buffer_sparse(struct sancus_buffer *b, size_t n)
{
	if (sancus_buffer_tail_size(b) < n)
		return -EINVAL;

	b->len += (uint_fast16_t)n;
	return (ssize_t)n;
}

ssize_t sancus_buffer__append(struct sancus_buffer *, bool, const char *, ssize_t);

static inline ssize_t sancus_buffer__appendz(struct sancus_buffer *b, bool truncate, const char *s)
{
	if (s) {
		ssize_t l = (ssize_t)strlen(s);
		return sancus_buffer__append(b, truncate, s, l);
	}
	return 0;
}

__attr_vprintf(3)
ssize_t sancus_buffer__appendv(struct sancus_buffer *b, bool, const char *fmt, va_list ap);

__attr_printf(3)
ssize_t sancus_buffer__appendf(struct sancus_buffer *, bool, const char *fmt, ...);

#define sancus_buffer_append(B, S, L)   sancus_buffer__append((B), false, (S), (L))
#define sancus_buffer_appendz(B, S)     sancus_buffer__appendz((B), false, (S))
#define sancus_buffer_appendv(B, F, AP) sancus_buffer__appendv((B), false, (F), (AP))
#define sancus_buffer_appendf(B, ...)   sancus_buffer__appendf((B), false, __VA_ARGS__)

#endif /* !__SANCUS_BUFFER_H__ */
