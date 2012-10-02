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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include "sancus_common.h"
#include "sancus_fd.h"
#include "sancus_socket.h"
#include "sancus_netlink.h"

/**
 * sancus_netlink_recvfrom - receive a netlink message
 */
static ssize_t sancus_netlink_recvfrom(int fd, void *buf, size_t buflen)
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

/* extract the netlink message from the buffer and call on_data callback */
static inline int extract_netlink_message(struct sancus_netlink_receiver *self,
				    struct ev_loop *loop,
				    const void *buf, size_t recvbytes)
{
	const struct sancus_netlink_receiver_settings *settings = self->settings;

	int ret = 0;
	int len = recvbytes;
	const struct nlmsghdr *nlh = buf;

	while (sancus_netlink_message_ok(nlh, len)) {
		/* check port id of netlink message against the one of the receiver */
		if (!sancus_netlink_message_pid_ok(nlh, self->pid)) {
			errno = ESRCH;
			return -1;
		}

		if (nlh->nlmsg_type >= NLMSG_MIN_TYPE) {
			ret = settings->on_data(self, loop, nlh);
			if (!ret) goto out;
		}
		nlh = sancus_netlink_message_next(nlh, &len);
	}

out:
	return ret;
}

/**
 * recv_cb - called when there is data received
 */
static void recv_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
	struct sancus_netlink_receiver *self = container_of(w, struct sancus_netlink_receiver,
						      recv_watcher);
	const struct sancus_netlink_receiver_settings *settings = self->settings;

	if (revents & EV_READ) {
		char buf[SANCUS_NETLINK_SOCKET_BUFFER_SIZE];
		int ret = sancus_netlink_recvfrom(w->fd, buf, sizeof(buf)); 
		if (ret < 0) {
			settings->on_error(self, loop,
					   SANCUS_NETLINK_RECEIVER_RECVFROM_ERROR);
		} else if (!extract_netlink_message(self, loop, buf, ret)) {
			sancus_close(&w->fd);
		}
	}

	if (revents & EV_ERROR) {
		sancus_netlink_receiver_stop(self, loop);
		sancus_netlink_receiver_close(self);

		settings->on_error(self, loop, SANCUS_NETLINK_RECEIVER_WATCHER_ERROR);
	}
}


/*
 * exported functions
 */

void sancus_netlink_receiver_start(struct sancus_netlink_receiver *self, struct ev_loop *loop)
{
	assert(!ev_is_active(&self->recv_watcher));
	ev_io_start(loop, &self->recv_watcher);
}

void sancus_netlink_receiver_stop(struct sancus_netlink_receiver *self, struct ev_loop *loop)
{
	assert(ev_is_active(&self->recv_watcher));
	ev_io_stop(loop, &self->recv_watcher);
}

void sancus_netlink_receiver_close(struct sancus_netlink_receiver *self)
{
	assert(self->recv_watcher.fd >= 0);
	assert(!ev_is_active(&self->recv_watcher));

	sancus_close(&self->recv_watcher.fd);
}

int sancus_netlink_receiver_listen(struct sancus_netlink_receiver *self,
			   const struct sancus_netlink_receiver_settings *settings,
			   int bus, unsigned int groups, pid_t pid)
{
	struct sockaddr_nl sa = {
		.nl_family = AF_NETLINK,
		.nl_groups = groups,
		.nl_pid = pid,
	};

	int fd = sancus_socket(AF_NETLINK, SOCK_RAW, bus, 0, true);
	if (fd < 0)
		return -1;

	assert(self);
	assert(settings);

	ev_io_init(&self->recv_watcher, recv_cb, fd, EV_READ);

	self->settings = settings;
	self->pid = pid;

	if (bind(fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) { 
		int e = errno;
		sancus_close(&self->recv_watcher.fd);
		errno = e;
		return -1;
	}

	return 1;
}

