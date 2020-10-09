#ifndef __SANCUS_BUFFER_H__
#define __SANCUS_BUFFER_H__

#include <stdarg.h>
#include <stdint.h>
#include <string.h>

struct sancus_buffer {
	char *buf;
	uint_fast16_t base, len, size;
};

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

ssize_t sancus_buffer_stripn(struct sancus_buffer *, size_t);

#define sancus_buffer_pop(B, N) sancus_buffer_stripn((B), (N))

/*
 * append
 */
ssize_t sancus_buffer_append(struct sancus_buffer *, const char *, ssize_t);

static inline ssize_t sancus_buffer_appendz(struct sancus_buffer *b, const char *s)
{
	if (s)
		return sancus_buffer_append(b, s, strlen(s));
	return 0;
}

__attr_vprintf(2)
ssize_t sancus_buffer_appendv(struct sancus_buffer *b, const char *fmt, va_list ap);

__attr_printf(2)
ssize_t sancus_buffer_appendf(struct sancus_buffer *, const char *fmt, ...);

#endif /* !__SANCUS_BUFFER_H__ */
