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
#include <sancus/fd.h>
#include <sancus/time.h>

#include <assert.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <arpa/inet.h>

#include <sancus/socket.h>
#include <sancus/tcp_conn.h>

static void io_cb(struct sancus_ev_loop *loop, struct sancus_ev_fd *w, int revents)
{
	struct sancus_tcp_conn *self = container_of(w, struct sancus_tcp_conn, io);
	const struct sancus_tcp_conn_settings *settings = self->settings;

	if (revents & SANCUS_EV_ERROR) {
		settings->on_error(self, loop, SANCUS_TCP_CONN_WATCHER_ERROR);
		return;
	}

	switch (self->state) {
	case SANCUS_TCP_CONN_INPROGRESS:
		assert(revents & SANCUS_EV_WRITE);

		if (revents & SANCUS_EV_READ) {
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

		/* fall-through */
	case SANCUS_TCP_CONN_CONNECTED:
		assert(revents & SANCUS_EV_WRITE);

connect_done:
		{
			/* only reads from now on */
			int fd = w->fd;
			sancus_ev_fd_stop (loop, w);
			sancus_ev_fd_init(w, io_cb, fd, SANCUS_EV_READ);
			sancus_ev_fd_start(loop, w);
		}

		settings->on_connect(self, loop);
		self->state = SANCUS_TCP_CONN_RUNNING;
		sancus_tcp_conn_touch(self, loop);
		break;
	case SANCUS_TCP_CONN_RUNNING:
		if (revents & SANCUS_EV_READ) {
			settings->on_read(self, loop);
			sancus_tcp_conn_touch(self, loop);
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

static inline int init_ipv6(struct sockaddr_in6 *sin6, const char *addr, unsigned port)
{
	sin6->sin6_port = htons(port);

	return inet_pton(sin6->sin6_family, addr, &sin6->sin6_addr);
}

static inline int init_local(struct sockaddr_un *sun, const char *path)
{
	size_t l = 0;

	if (path == NULL) {
		path = "";
	} else {
		l = strlen(path);
		if (l > sizeof(sun->sun_path)-1)
			return 0; /* too long */
	}
	memcpy(sun->sun_path, path, l+1);
	return 1;
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

	sancus_ev_fd_init(&self->io, io_cb, fd, SANCUS_EV_READ|SANCUS_EV_WRITE);

	self->settings = settings;

	if (connect(fd, sa, sa_len) < 0) {
		if (errno == EINPROGRESS) {
			self->state = SANCUS_TCP_CONN_INPROGRESS;
		} else {
			int e = errno;
			sancus_close2(&self->io.fd);
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
void sancus_tcp_conn_start(struct sancus_tcp_conn *self, struct sancus_ev_loop *loop)
{
	if (!sancus_ev_is_active(&self->io)) {

		sancus_ev_fd_start(loop, &self->io);

		if (sancus_time_is_zero(&self->last_activity))
			sancus_tcp_conn_touch(self, loop);
	}
}

void sancus_tcp_conn_stop(struct sancus_tcp_conn *self, struct sancus_ev_loop *loop)
{
	if (sancus_ev_is_active(&self->io))
		sancus_ev_fd_stop(loop, &self->io);
}

void sancus_tcp_conn_close(struct sancus_tcp_conn *self)
{
	assert(self->io.fd >= 0);
	assert(!sancus_ev_is_active(&self->io));

	sancus_close2(&self->io.fd);
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

int sancus_tcp_ipv6_connect(struct sancus_tcp_conn *self,
			    const struct sancus_tcp_conn_settings *settings,
			    const char *addr, unsigned port,
			    bool cloexec)
{
	struct sockaddr_in6 sin6 = { .sin6_family = AF_INET6 };
	int e;

	if ((e = init_ipv6(&sin6, addr, port)) != 1)
		return e;

	return init_tcp(self, settings, (struct sockaddr *)&sin6, sizeof(sin6),
			cloexec);
}

int sancus_tcp_local_connect(struct sancus_tcp_conn *self,
			     const struct sancus_tcp_conn_settings *settings,
			     const char *path, bool cloexec)
{
	struct sockaddr_un sun = { .sun_family = AF_LOCAL };
	int e;

	if ((e = init_local(&sun, path)) != 1)
		return e;

	return init_tcp(self, settings, (struct sockaddr *)&sun, sizeof(sun),
			cloexec);
}
