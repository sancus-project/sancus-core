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
#ifndef _SANCUS_NETLINK_H
#define _SANCUS_NETLINK_H

struct sancus_nl_receiver;

/**
 * enum sancus_nl_receiver_error - list of possible errors
 *
 * @SANCUS_NL_RECEIVER_WATCHER_ERROR:	receive watcher error, server has been closed
 */
enum sancus_nl_receiver_error {
	SANCUS_NL_RECEIVER_WATCHER_ERROR,
	SANCUS_NL_RECEIVER_RECVFROM_ERROR,
};

/**
 * struct sancus_receiver_settings - driving callbacks of netlink receiver
 *
 * @on_data:	new data received, return %false if it should be closed
 * @on_error:	an error has happened, tell the world
 */
struct sancus_nl_receiver_settings {
	bool (*on_data) (struct sancus_nl_receiver *, struct ev_loop *, const struct nlmsghdr *);

	void (*on_error) (struct sancus_nl_receiver *,
			  struct ev_loop *,
			  enum sancus_nl_receiver_error);
};

/**
 * struct sancus_nl_receiver - netlink receiver
 *
 * @recv_watcher:	receive watcher
 * @settings:		driving callbacks
 * @pid:		port ID
 */
struct sancus_nl_receiver {
	struct ev_io recv_watcher;

	const struct sancus_nl_receiver_settings *settings;

	pid_t pid;
};

/**
 * sancus_nl_receiver_start - start watching netlink socket
 *
 * @self:	server to be started
 * @loop:	event loop
 *
 * The server shall not be already active. Nothing returned
 */
void sancus_nl_receiver_start(struct sancus_nl_receiver *self, struct ev_loop *loop);

/**
 * sancus_nl_receiver_stop - stop watching netlink socket
 * @self:	server to be stopped
 * @loop:	event loop
 *
 * The server shall not be already stopped. Nothing returned
 */
void sancus_nl_receiver_stop(struct sancus_nl_receiver *self, struct ev_loop *loop);

/**
 * sancus_nl_receiver_close - closes an already stopped netlink receiver
 *
 * @self:	server to close
 */
void sancus_nl_receiver_close(struct sancus_nl_receiver *self);

/**
 * sancus_nl_receiver_listen - initializes and prepares netlink receiver
 *
 * @self:	server structure to initialize
 * @settings:	driving callbacks
 * @bus:	netlink socket bus ID (see NETLINK_* constants)	
 * @groups:	the group of message you're interested in
 * @pid:	port ID you want to use (use zero for automatic selection)
 *
 * Returns 0 if @addr is invalid, 1 on success and -1 on error. errno set
 * accordingly.
 */
int sancus_nl_receiver_listen(struct sancus_nl_receiver *self,
			   const struct sancus_nl_receiver_settings *settings,
			   int bus, unsigned int groups, pid_t pid);
/**
 * SANCUS_NL_SOCKET_BUFFER_SIZE - netlink socket buffer size
 */
#define SANCUS_NL_SOCKET_BUFFER_SIZE (getpagesize() < 8192L ? getpagesize() : 8192L)

/**
 * sancus_nl_receiver_fd - returns fd been listened by a netlink receiver
 *
 * @S:	server structure to query
 */
#define sancus_nl_receiver_fd(S)		(S)->recv_watcher.fd

/**
 * sancus_nl_receiver_is_active - checks if a server has not been closed nor stoped
 *
 * @S:	server structure to query
 */
#define sancus_nl_receiver_is_active(S)	((S)->recv_watcher.fd > 0 && ev_is_active(&(S)->recv_watcher))



/**
 * Netlink Message handling
 */

/* Netlink message headers and its attributes are always aligned to four bytes. */
#define SANCUS_NL_ALIGNTO		4
#define SANCUS_NL_ALIGN(len)	(((len)+SANCUS_NL_ALIGNTO-1) & ~(SANCUS_NL_ALIGNTO-1))
#define SANCUS_NL_MSG_HDRLEN	SANCUS_NL_ALIGN(sizeof(struct nlmsghdr))

/**
 * sancus_nl_msg_ok - check if there is room for a netlink message
 * @nlh:	netlink message that we want to check
 * @len:	remaining bytes in a buffer that contains the netlink message
 */
bool sancus_nl_msg_ok(const struct nlmsghdr *nlh, int len);

/**
 * sancus_nl_msg_next - get the next netlink message out of a multipart message
 * @nlh:	netlink message that is currently handled
 * @len:	length of the remaining bytes in the buffer
 */
struct nlmsghdr *sancus_nl_msg_next(const struct nlmsghdr *nlh, int *len);

/**
 * sancus_nl_msg_pid_ok - perform a check if sending port ID is correct
 * @nlh:	netlink message that is currently handled
 * @pid:	netlink portid to check
 */
bool sancus_nl_msg_pid_ok(const struct nlmsghdr *nlh, unsigned int pid);

/**
 * sancus_nl_msg_get_payload - get a pointer to the payload of the netlink message
 * @nlh:	netlink message to get the payload from
 */
void *sancus_nl_msg_get_payload(const struct nlmsghdr *nlh);


/**
 * Netlink Attribute handling
 */

/**
 * sancus_nl_attr_get_type - get the type of a netlink attribute
 * @attr:	pointer to a netlink attribute
 */
uint16_t sancus_nl_attr_get_type(const struct nlattr *attr);

/**
 * sancus_nl_attr_get_len - get the length of a netlink attribute
 * @attr:	pointer to a netlink attribute
 */
uint16_t sancus_nl_attr_get_len(const struct nlattr *attr);

#endif /* !_SANCUS_NETLINK_H */
