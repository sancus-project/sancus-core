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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <arpa/inet.h>

#include "sancus_common.h"
#include "sancus_fd.h"
#include "sancus_socket.h"
#include "sancus_tcp_conn.h"

static void io_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
	struct sancus_tcp_conn *self = container_of(w, struct sancus_tcp_conn, io);
	const struct sancus_tcp_conn_settings *settings = self->settings;

	if (revents & EV_ERROR) {
		settings->on_error(self, loop, SANCUS_TCP_CONN_WATCHER_ERROR);
		return;
	}

	switch (self->state) {
	case SANCUS_TCP_CONN_INPROGRESS:
		assert(revents & EV_WRITE);

		if (revents & EV_READ) {
			/* failed to connected? */
			int error;
			socklen_t len = sizeof(error);

			if (getsockopt(sancus_tcp_conn_fd(self),
				       SOL_SOCKET, SO_ERROR, &error, &len) < 0)
				; /* connect failed but errno already set. */
			else if (error == 0)
				goto connect_done;
			else
				errno = error;

			/* might want to retry */
			self->state = SANCUS_TCP_CONN_FAILED;
			settings->on_error(self, loop, SANCUS_TCP_CONN_CONNECT_ERROR);
			return;
		}

connect_done:
		; /* pass thru */
	case SANCUS_TCP_CONN_CONNECTED:
		assert(revents & EV_WRITE);

		{
			/* only reads from now on */
			int fd = w->fd;
			ev_io_stop (loop, w);
			ev_io_init(w, io_cb, fd, EV_READ);
			ev_io_start(loop, w);
		}

		settings->on_connect(self, loop);
		self->state = SANCUS_TCP_CONN_RUNNING;
		self->state_time = ev_now(loop);
		break;
	case SANCUS_TCP_CONN_RUNNING:
		if (revents & EV_READ) {
			settings->on_read(self, loop);
			self->state_time = ev_now(loop);
		}
		break;
	case SANCUS_TCP_CONN_FAILED:
		assert(0); /* fix your app! */
	}
}

/*
 * init helpers
 */
static inline int init_ipv4(struct sockaddr_in *sin, const char *addr, unsigned port)
{
	sin->sin_port = htons(port);

	return inet_pton(sin->sin_family, addr, &sin->sin_addr);
}

static inline int init_tcp(struct sancus_tcp_conn *self,
			   const struct sancus_tcp_conn_settings *settings,
			   struct sockaddr *sa, socklen_t sa_len,
			   bool cloexec)
{
	int fd = sancus_socket(sa->sa_family, SOCK_STREAM, 0, cloexec, true);
	if (fd < 0)
		return -1;

	assert(self);
	assert(settings);

	ev_io_init(&self->io, io_cb, fd, EV_READ|EV_WRITE);

	self->settings = settings;

	if (connect(fd, sa, sa_len) < 0) {
		if (errno == EINPROGRESS) {
			self->state = SANCUS_TCP_CONN_INPROGRESS;
		} else {
			int e = errno;
			sancus_close(&self->io.fd);
			errno = e;
			return -1;
		}
	} else {
		self->state = SANCUS_TCP_CONN_CONNECTED;
	}

	return 1;
}

/*
 * exported functions
 */
void sancus_tcp_conn_start(struct sancus_tcp_conn *self, struct ev_loop *loop)
{
	assert(!ev_is_active(&self->io));
	ev_io_start(loop, &self->io);

	if (self->state_time == 0.0)
		self->state_time = ev_now(loop);
}

void sancus_tcp_conn_stop(struct sancus_tcp_conn *self, struct ev_loop *loop)
{
	assert(ev_is_active(&self->io));
	ev_io_stop(loop, &self->io);
}

void sancus_tcp_conn_close(struct sancus_tcp_conn *self)
{
	assert(self->io.fd >= 0);
	assert(!ev_is_active(&self->io));

	sancus_close(&self->io.fd);
}

int sancus_tcp_ipv4_connect(struct sancus_tcp_conn *self,
			    const struct sancus_tcp_conn_settings *settings,
			    const char *addr, unsigned port,
			    bool cloexec)
{
	struct sockaddr_in sin = { .sin_family = AF_INET };
	int e;

	if ((e = init_ipv4(&sin, addr, port)) != 1)
		return e;

	return init_tcp(self, settings, (struct sockaddr *)&sin, sizeof(sin),
			cloexec);
}
