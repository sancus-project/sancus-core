#ifndef __SANCUS_BUFFER_LOCAL__
#define __SANCUS_BUFFER_LOCAL__

#include <sancus/buffer.h>

#define DECL_SANCUS_LBUFFER(N, S) struct { \
	char buf[S]; \
	struct sancus_buffer b; \
} N = { .b = { \
	.buf = (N).buf, \
	.size = (S), \
}, }

/*
 * accessors
 */
#define sancus_lbuffer_to_buffer(B) ((B) != NULL ? &(B)->b : NULL)

#define sancus_lbuffer_ptr(B) sancus_buffer_ptr(sancus_lbuffer_to_buffer(B))
#define sancus_lbuffer_len(B) sancus_buffer_len(sancus_lbuffer_to_buffer(B))

#define sancus_lbuffer_pop(B, N) sancus_buffer_pop(sancus_lbuffer_to_buffer(B), (N))

/*
 * append
 */
#define sancus_lbuffer_append(B,  S, N) sancus_buffer_append(sancus_lbuffer_to_buffer(B),  (S), (N))
#define sancus_lbuffer_appendz(B, S)    sancus_buffer_appendz(sancus_lbuffer_to_buffer(B), (S))
#define sancus_lbuffer_appendf(B, ...)  sancus_buffer_appendf(sancus_lbuffer_to_buffer(B), __VA_ARGS__)
#define sancus_lbuffer_appendv(B, ...)  sancus_buffer_appendv(sancus_lbuffer_to_buffer(B), __VA_ARGS__)

#endif /* !__SANCUS_BUFFER_LOCAL__ */
