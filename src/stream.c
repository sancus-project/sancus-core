/*
 * Copyright (c) 2011-2012, Alejandro Mery <amery@geeks.cl>
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

#include <sancus/common.h>
#include <sancus/ev.h>

#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>

#include <fcntl.h>

#include <sancus/fd.h>
#include <sancus/buffer.h>
#include <sancus/stream.h>

/*
 * event callbacks
 */
static void read_cb(struct sancus_ev_loop *loop, struct sancus_ev_fd *w, int revents)
{
	struct sancus_stream *self = container_of(w, struct sancus_stream, read_watcher);
	struct sancus_stream_settings *settings = self->settings;

	assert((revents & SANCUS_EV_ERROR) == 0);

	if (revents & SANCUS_EV_READ) {
		struct sancus_buffer *buf = &self->read_buffer;
		ssize_t l;

read_buffer_available:
		if (!sancus_buffer_available(buf)) {
			if (!settings->on_error(self, loop,
						SANCUS_STREAM_READ_FULL))
				goto read_buffer_available;
			else
				goto close_stream;
		}

try_read:
		l = sancus_buffer_read(buf, w->fd);
		if (l > 0) {
			while ((l = sancus_buffer_len(buf))) {
				l = settings->on_read(self, sancus_buffer_data(buf), l);
				if (l > 0) {
					sancus_buffer_skip(buf, l);
					if (!sancus_ev_is_active(w))
						break;
				} else if (l == 0)
					break;
				else
					goto close_stream;
			}
		} else if (l == 0) {
			if (settings->on_error(self, loop, SANCUS_STREAM_READ_EOF))
				goto close_stream;
		} else if (errno == EINTR) {
			goto try_read;
		} else if (errno != EAGAIN && errno != EWOULDBLOCK) {
			if (settings->on_error(self, loop, SANCUS_STREAM_READ_ERROR))
				goto close_stream;
		}

	}

	return;

close_stream:
	sancus_stream_stop(self, loop);
	sancus_stream_close(self);
}

/*
 * exported functions
 */
ssize_t sancus_stream_process(struct sancus_stream *self)
{
	struct sancus_stream_settings *settings = self->settings;
	struct sancus_buffer *buf = &self->read_buffer;
	ssize_t l = sancus_buffer_len(buf);

	if (l > 0) {
		l = settings->on_read(self, sancus_buffer_data(buf), l);
		if (l > 0)
			sancus_buffer_skip(buf, l);
	}

	return l;
}

void sancus_stream_start(struct sancus_stream *self, struct sancus_ev_loop *loop)
{
	assert(!sancus_ev_is_active(&self->read_watcher));

	sancus_ev_fd_start(loop, &self->read_watcher);
}

void sancus_stream_stop(struct sancus_stream *self, struct sancus_ev_loop *loop)
{
	assert(sancus_ev_is_active(&self->read_watcher));

	sancus_ev_fd_stop(loop, &self->read_watcher);

}

void sancus_stream_close(struct sancus_stream *self)
{
	assert(self->read_watcher.fd >= 0);
	assert(!sancus_ev_is_active(&self->read_watcher));

	sancus_close(&self->read_watcher.fd);
	self->settings->on_close(self);
}

int sancus_stream_init(struct sancus_stream *self,
		       struct sancus_stream_settings *settings,
		       int fd,
		       char *read_buffer, size_t read_buf_size)
{
	assert(self);
	assert(settings);
	assert(settings->on_error);
	assert(settings->on_close);

	assert(fd >= 0);
	assert(!read_buffer || read_buf_size > 0);

	self->settings = settings;

	sancus_ev_fd_init(&self->read_watcher, read_cb, fd, SANCUS_EV_READ);

	sancus_buffer_bind(&self->read_buffer, read_buffer, read_buf_size);

	return 1;
}
