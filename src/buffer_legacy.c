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
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <sancus/buffer_legacy.h>
#include <sancus/fd.h>

void sancus_buffer_bind(struct sancus_buffer *self, char *buf, size_t size)
{
	assert((buf && size > 0) || (!buf && size == 0));

	*self = (struct sancus_buffer) {
		.buf = buf,
		.size = size,
	};
}

ssize_t sancus_buffer_read(struct sancus_buffer *self, int fd)
{
	ssize_t l = sancus_read(fd, sancus_buffer_data(self)+sancus_buffer_len(self),
				sancus_buffer_available(self));
	if (l > 0)
		self->len += l;

	return l;
}

void sancus_buffer_rebase(struct sancus_buffer *self)
{
	if (self->base > 0) {
		if (self->len > 0) {
			memmove(self->buf,
				self->buf+self->base,
				self->len);
		}
		self->base = 0;
	}
}

size_t sancus_buffer_skip(struct sancus_buffer *self, size_t step)
{
	if (step >= self->len) {
		sancus_buffer_reset(self);
		return self->size;
	} else if (step > 0) {
		self->base += step;
		self->len -= step;
	}

	/* if there is less than 10% available, try to rebase */
	if (sancus_buffer_available(self) < (self->size / 10))
		sancus_buffer_rebase(self);

	return sancus_buffer_available(self);
}
