/*
 * Copyright (c) 2012, Christian Wiese <chris@opensde.org>
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
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <sancus/fd.h>
#include <sancus/socket.h>
#include <sancus/netlink.h>

/**
 * sancus_nl_recvfrom - receive a netlink message
 */
static ssize_t sancus_nl_recvfrom(int fd, void *buf, size_t buflen)
{
	ssize_t ret;
	struct sockaddr_nl addr;
	struct iovec iov = {
		.iov_base       = buf,
		.iov_len        = buflen,
	};
	struct msghdr msg = {
		.msg_name       = &addr,
		.msg_namelen    = sizeof(struct sockaddr_nl),
		.msg_iov        = &iov,
		.msg_iovlen     = 1,
		.msg_control    = NULL,
		.msg_controllen = 0,
		.msg_flags      = 0,
	};
	ret = recvmsg(fd, &msg, 0);
	if (ret == -1)
		return ret;

	if (msg.msg_flags & MSG_TRUNC) {
		errno = ENOSPC;
		return -1;
	}
	if (msg.msg_namelen != sizeof(struct sockaddr_nl)) {
		errno = EINVAL;
		return -1;
	}
	return ret;
}

/* extract the netlink message from the buffer and call on_message callback */
static inline int extract_netlink_message(struct sancus_nl_receiver *self,
				    struct sancus_ev_loop *loop,
				    const void *buf, size_t recvbytes)
{
	const struct sancus_nl_receiver_settings *settings = self->settings;

	int ret = 0;
	int len = recvbytes;
	const struct nlmsghdr *nlh = buf;

	while (sancus_nl_msg_ok(nlh, len)) {
		/* check port id of netlink message against the one of the receiver */
		if (!sancus_nl_msg_portid_ok(nlh, self->portid)) {
			errno = ESRCH;
			return -1;
		}

		if (nlh->nlmsg_type >= NLMSG_MIN_TYPE) {
			ret = settings->on_message(self, loop, nlh);
			if (!ret) goto out;
		}
		nlh = sancus_nl_msg_next(nlh, &len);
	}

out:
	return ret;
}

/**
 * recv_cb - called when there is data received
 */
static void recv_cb(struct sancus_ev_loop *loop, struct sancus_ev_fd *w, int revents)
{
	struct sancus_nl_receiver *self = container_of(w, struct sancus_nl_receiver,
						      recv_watcher);
	const struct sancus_nl_receiver_settings *settings = self->settings;

	if (revents & SANCUS_EV_READ) {
		char buf[SANCUS_NL_SOCKET_BUFFER_SIZE];
		int ret = sancus_nl_recvfrom(w->fd, buf, sizeof(buf)); 
		if (ret < 0) {
			settings->on_error(self, loop,
					   SANCUS_NL_RECEIVER_RECVFROM_ERROR);
		} else if (!extract_netlink_message(self, loop, buf, ret)) {
			sancus_close2(&w->fd);
		}
	}

	if (revents & SANCUS_EV_ERROR) {
		sancus_nl_receiver_stop(self, loop);
		sancus_nl_receiver_close(self);

		settings->on_error(self, loop, SANCUS_NL_RECEIVER_WATCHER_ERROR);
	}
}


/*
 * exported functions
 */

void sancus_nl_receiver_start(struct sancus_nl_receiver *self, struct sancus_ev_loop *loop)
{
	assert(!sancus_ev_is_active(&self->recv_watcher));
	sancus_ev_fd_start(loop, &self->recv_watcher);
}

void sancus_nl_receiver_stop(struct sancus_nl_receiver *self, struct sancus_ev_loop *loop)
{
	assert(sancus_ev_is_active(&self->recv_watcher));
	sancus_ev_fd_stop(loop, &self->recv_watcher);
}

void sancus_nl_receiver_close(struct sancus_nl_receiver *self)
{
	assert(self->recv_watcher.fd >= 0);
	assert(!sancus_ev_is_active(&self->recv_watcher));

	sancus_close2(&self->recv_watcher.fd);
}

int sancus_nl_receiver_listen(struct sancus_nl_receiver *self,
			   const struct sancus_nl_receiver_settings *settings,
			   int bus, unsigned int groups, pid_t portid)
{
	struct sockaddr_nl sa = {
		.nl_family = AF_NETLINK,
		.nl_groups = groups,
		.nl_pid = portid,
	};

	int fd = sancus_socket(AF_NETLINK, SOCK_RAW, bus, 0, true);
	if (fd < 0)
		return -1;

	assert(self);
	assert(settings);

	sancus_ev_fd_init(&self->recv_watcher, recv_cb, fd, SANCUS_EV_READ);

	self->settings = settings;
	self->portid = portid;

	if (bind(fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) { 
		int e = errno;
		sancus_close2(&self->recv_watcher.fd);
		errno = e;
		return -1;
	}

	return 1;
}

