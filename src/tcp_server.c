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
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include "sancus_fd.h"
#include "sancus_socket.h"
#include "sancus_tcp_server.h"

/*
 * init helpers
 */
static inline int init_ipv4(struct sockaddr_in *sin, const char *addr, unsigned port)
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

static inline int init_tcp(struct sancus_tcp_listener *self,
			   struct sancus_tcp_server_settings *settings,
			   struct sockaddr *sa, socklen_t sa_len,
			   bool cloexec, unsigned backlog)
{
	int fd = sancus_socket(sa->sa_family, SOCK_STREAM, cloexec, true);
	if (fd < 0)
		return -1;

	assert(self);
	assert(settings);

	{
		int flags = 1;
		struct linger ling = {0, 0}; /* disabled */

		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&flags, sizeof(flags));
		setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&flags, sizeof(flags));
		setsockopt(fd, SOL_SOCKET, SO_LINGER, (void*)&ling, sizeof(ling));
	}

	if (bind(fd, sa, sa_len) < 0 ||
	    listen(fd, backlog) < 0) {
		int e = errno;
		sancus_close(&fd);
		errno = e;
		return -1;
	}

	self->fd = fd;
	self->settings = settings;

	return 1;
}

/*
 * exported functions
 */
int sancus_tcp_ipv4_listen(struct sancus_tcp_listener *self,
			   struct sancus_tcp_server_settings *settings,
			   const char *addr, unsigned port,
			   bool cloexec, unsigned backlog)
{
	struct sockaddr_in sin = { .sin_family = AF_INET };
	int e;

	if ((e = init_ipv4(&sin, addr, port)) != 1)
	    return e;

	return init_tcp(self, settings, (struct sockaddr *)&sin, sizeof(sin),
			cloexec, backlog);
}
