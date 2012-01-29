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
 *
 * Alternatively, the contents of this package may be used under the terms
 * of the GNU General Public License ("GPL") version 2 or any later version,
 * in which case the provisions of the GPL are applicable instead of the
 * above. If you wish to allow the use of your version of this package only
 * under the terms of the GPL and not to allow others to use your version of
 * this file under the BSD license, indicate your decision by deleting the
 * provisions above and replace them with the notice and other provisions
 * required by the GPL in this and the other files of this package. If you do
 * not delete the provisions above, a recipient may use your version of this
 * file under either the BSD or the GPL.
 */

#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <fcntl.h>

#include "sancus_common.h"
#include "sancus_fd.h"
#include "sancus_buffer.h"
#include "sancus_stream.h"

/*
 * event callbacks
 */
static void read_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
	struct sancus_stream *self = container_of(w, struct sancus_stream, read_watcher);
	struct sancus_stream_settings *settings = self->settings;

	if (revents & EV_READ) {
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
					if (!ev_is_active(w))
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

	if (revents & EV_ERROR) {
		settings->on_error(self, loop, SANCUS_STREAM_READ_WATCHER_ERROR);

close_stream:
		sancus_stream_stop(self, loop);
		sancus_stream_close(self);
	}
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

void sancus_stream_start(struct sancus_stream *self, struct ev_loop *loop)
{
	assert(!ev_is_active(&self->read_watcher));
	ev_io_start(loop, &self->read_watcher);
}

void sancus_stream_stop(struct sancus_stream *self, struct ev_loop *loop)
{
	assert(ev_is_active(&self->read_watcher));
	ev_io_stop(loop, &self->read_watcher);

}

void sancus_stream_close(struct sancus_stream *self)
{
	assert(self->read_watcher.fd >= 0);

	assert(!ev_is_active(&self->read_watcher));

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

	ev_io_init(&self->read_watcher, read_cb, fd, EV_READ);

	sancus_buffer_bind(&self->read_buffer, read_buffer, read_buf_size);

	return 1;
}
