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

struct sancus_netlink_receiver;

/**
 * enum sancus_netlink_receiver_error - list of possible errors
 *
 * @SANCUS_NETLINK_RECEIVER_WATCHER_ERROR:	receive watcher error, server has been closed
 */
enum sancus_netlink_receiver_error {
	SANCUS_NETLINK_RECEIVER_WATCHER_ERROR,
	SANCUS_NETLINK_RECEIVER_RECVFROM_ERROR,
};

/**
 * struct sancus_receiver_settings - driving callbacks of netlink receiver
 *
 * @on_data:	new data received, return %false if it should be closed
 * @on_error:	an error has happened, tell the world
 */
struct sancus_netlink_receiver_settings {
	bool (*on_data) (struct sancus_netlink_receiver *, struct ev_loop *, void *);

	void (*on_error) (struct sancus_netlink_receiver *,
			  struct ev_loop *,
			  enum sancus_netlink_receiver_error);
};

/**
 * struct sancus_netlink_receiver - netlink receiver
 *
 * @recv_watcher:	receive watcher
 * @settings:		driving callbacks
 */
struct sancus_netlink_receiver {
	struct ev_io recv_watcher;

	const struct sancus_netlink_receiver_settings *settings;
};

/**
 * sancus_netlink_receiver_start - start watching netlink socket
 *
 * @self:	server to be started
 * @loop:	event loop
 *
 * The server shall not be already active. Nothing returned
 */
void sancus_netlink_receiver_start(struct sancus_netlink_receiver *self, struct ev_loop *loop);

/**
 * sancus_netlink_receiver_stop - stop watching netlink socket
 * @self:	server to be stopped
 * @loop:	event loop
 *
 * The server shall not be already stopped. Nothing returned
 */
void sancus_netlink_receiver_stop(struct sancus_netlink_receiver *self, struct ev_loop *loop);

/**
 * sancus_netlink_receiver_close - closes an already stopped netlink receiver
 *
 * @self:	server to close
 */
void sancus_netlink_receiver_close(struct sancus_netlink_receiver *self);

/**
 * sancus_netlink_receiver_listen - initializes and prepares netlink receiver
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
int sancus_netlink_receiver_listen(struct sancus_netlink_receiver *self,
			   const struct sancus_netlink_receiver_settings *settings,
			   int bus, unsigned int groups, pid_t pid);
/**
 * SANCUS_NETLINK_SOCKET_BUFFER_SIZE - netlink socket buffer size
 */
#define SANCUS_NETLINK_SOCKET_BUFFER_SIZE (getpagesize() < 8192L ? getpagesize() : 8192L)

/**
 * sancus_netlink_receiver_fd - returns fd been listened by a netlink receiver
 *
 * @S:	server structure to query
 */
#define sancus_netlink_receiver_fd(S)		(S)->recv_watcher.fd

/**
 * sancus_netlink_receiver_is_active - checks if a server has not been closed nor stoped
 *
 * @S:	server structure to query
 */
#define sancus_netlink_receiver_is_active(S)	((S)->recv_watcher.fd > 0 && ev_is_active(&(S)->recv_watcher))

#endif /* !_SANCUS_NETLINK_H */