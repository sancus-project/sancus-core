/*
 * Copyright (c) 2011, Alejandro Mery <amery@geeks.cl>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _SANCUS_BUFFER_H
#define _SANCUS_BUFFER_H

/**
 * struct sancus_buffer - buffer control structure
 *
 * @buf:	pointer to the data buffer
 * @base:	offset to the base of the stored data
 * @len:	lenght of the stored data
 * @size:	size of the data buffer
 */
struct sancus_buffer {
	char *buf;

	uint_fast16_t base, len, size;
};

/**
 * sancus_buffer_bind - binds an static bytes array to a buffer struct
 */
void sancus_buffer_bind(struct sancus_buffer *self, char *buf, size_t size);

/**
 * sancus_buffer_available - tells how much free space lefts in the tail of buffer
 */
#define sancus_buffer_available(B)	((B)->size - (B)->base - (B)->len)

/**
 * sancus_buffer_len - amount of data in the buffer
 */
#define sancus_buffer_len(B)	((B)->len)

/**
 * sancus_buffer_data - pointer to the data in the buffer
 */
#define sancus_buffer_data(B)	((B)->buf + (B)->base)

/**
 * sancus_buffer_read - read from fd to buffer
 */
ssize_t sancus_buffer_read(struct sancus_buffer *, int fd);

/**
 * sancus_buffer_rebase - moves data to the head of the buffer
 */
void sancus_buffer_rebase(struct sancus_buffer *);

/**
 * sancus_buffer_skip - drop the first n chars from the buffer
 */
size_t sancus_buffer_skip(struct sancus_buffer *, size_t);

/**
 * sancus_buffer_reset - trashes all data in the buffer
 */
#define sancus_buffer_reset(B)	do { (B)->base = (B)->len = 0; } while(0)

#endif /* !_SANCUS_BUFFER_H */
