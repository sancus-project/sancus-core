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

#include <sancus/common.h>
#include <sancus/ev.h>
#include <sancus/fd.h>

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <arpa/inet.h>

#include <sancus/socket.h>
#include <sancus/tcp_server.h>

/**
 * connect_cb - called when there is incoming
 */
static void connect_cb(struct sancus_ev_loop *loop, struct sancus_ev_fd *w, int revents)
{
	struct sancus_tcp_server *self = container_of(w, struct sancus_tcp_server,
						      connect);
	const struct sancus_tcp_server_settings *settings = self->settings;

	assert(!(revents & SANCUS_EV_ERROR));

	if (revents & SANCUS_EV_READ) {
		struct sockaddr_storage addr;
		socklen_t addrlen = sizeof(addr);

		int fd = sancus_accept(w->fd, (struct sockaddr*)&addr, &addrlen);
		if (fd < 0) {
			settings->on_error(self, loop,
					   SANCUS_TCP_SERVER_ACCEPT_ERROR);
		} else if (!settings->on_connect(self, loop, fd,
						 (struct sockaddr*)&addr, addrlen)) {
			sancus_close2(&fd);
		}
	}
}

/*
 * init helpers
 */
static inline int init_ipv4(struct sockaddr_in *sin, const char *addr, uint16_t port)
{
	sin->sin_port = htons(port);

	/* NULL, "", "0" and "*" mean any address */
	if (addr == NULL || addr[0] == '\0' ||
	    ((addr[0] == '0' || addr[0] == '*') && addr[1] == '\0')) {
		sin->sin_addr.s_addr = htonl(INADDR_ANY);
		return 1;
	}

	return inet_pton(sin->sin_family, addr, &sin->sin_addr);
}

static inline int init_ipv6(struct sockaddr_in6 *sin6, const char *addr, uint16_t port)
{
	sin6->sin6_port = htons(port);

	/* NULL, "", "*" and "::" mean any address */
	if (addr == NULL || addr[0] == '\0' ||
	    (addr[0] == '*' && addr[1] == '\0') ||
	    (addr[0] == ':' && addr[1] == ':' && addr[2] == '\0')) {
		sin6->sin6_addr = in6addr_any;
		return 1;
	}

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

static inline int init_tcp(struct sancus_tcp_server *self,
			   const struct sancus_tcp_server_settings *settings,
			   struct sockaddr *sa, socklen_t sa_len,
			   bool cloexec, unsigned backlog)
{
	int fd = sancus_socket(sa->sa_family, SOCK_STREAM, 0, cloexec, true);
	if (fd < 0)
		return -1;

	assert(self);
	assert(settings);

	if (sa->sa_family != AF_LOCAL) {
		int flags = 1;
		struct linger ling = {0, 0}; /* disabled */

		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&flags, sizeof(flags));
		setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&flags, sizeof(flags));
		setsockopt(fd, SOL_SOCKET, SO_LINGER, (void*)&ling, sizeof(ling));
	}

	sancus_ev_fd_init(&self->connect, connect_cb, fd, SANCUS_EV_READ);
	/* TODO: any ->data to add? */

	self->settings = settings;

	if (settings->pre_bind)
		settings->pre_bind(self);

	if (bind(fd, sa, sa_len) < 0 ||
	    listen(fd, (int)backlog) < 0) {
		int e = errno;
		sancus_close2(&self->connect.fd);
		errno = e;
		return -1;
	}

	return 1;
}

/*
 * exported functions
 */

void sancus_tcp_server_start(struct sancus_tcp_server *self, struct sancus_ev_loop *loop)
{
	assert(!sancus_ev_is_active(&self->connect));
	sancus_ev_fd_start(loop, &self->connect);
}

void sancus_tcp_server_stop(struct sancus_tcp_server *self, struct sancus_ev_loop *loop)
{
	if (sancus_ev_is_active(&self->connect))
		sancus_ev_fd_stop(loop, &self->connect);
}

void sancus_tcp_server_close(struct sancus_tcp_server *self)
{
	assert(self->connect.fd >= 0);
	assert(!sancus_ev_is_active(&self->connect));

	sancus_close2(&self->connect.fd);
}

int sancus_tcp_ipv4_listen(struct sancus_tcp_server *self,
			   const struct sancus_tcp_server_settings *settings,
			   const char *addr, uint16_t port,
			   bool cloexec, unsigned backlog)
{
	struct sockaddr_in sin = { .sin_family = AF_INET };
	int e;

	if ((e = init_ipv4(&sin, addr, port)) != 1)
		return e;

	return init_tcp(self, settings, (struct sockaddr *)&sin, sizeof(sin),
			cloexec, backlog);
}

int sancus_tcp_ipv6_listen(struct sancus_tcp_server *self,
			   const struct sancus_tcp_server_settings *settings,
			   const char *addr, uint16_t port,
			   bool cloexec, unsigned backlog)
{
	struct sockaddr_in6 sin6 = { .sin6_family = AF_INET6 };
	int e;

	if ((e = init_ipv6(&sin6, addr, port)) != 1)
		return e;

	return init_tcp(self, settings, (struct sockaddr *)&sin6, sizeof(sin6),
			cloexec, backlog);
}

int sancus_tcp_local_listen(struct sancus_tcp_server *self,
			    const struct sancus_tcp_server_settings *settings,
			    const char *path,
			    bool cloexec, unsigned backlog)
{
	struct sockaddr_un sun = { .sun_family = AF_LOCAL };
	int e;

	if ((e = init_local(&sun, path)) != 1)
		return e;
	unlink(path);

	return init_tcp(self, settings, (struct sockaddr *)&sun, sizeof(sun),
			cloexec, backlog);
}
