/*
 * Copyright (c) 2012, Alejandro Mery <amery@geeks.cl>
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
#ifndef _SANCUS_TCP_CLIENT_H
#define _SANCUS_TCP_CLIENT_H

struct sancus_tcp_conn;

enum sancus_tcp_conn_error {
	SANCUS_TCP_CONN_WATCHER_ERROR,
	SANCUS_TCP_CONN_CONNECT_ERROR,
};

enum sancus_tcp_conn_state {
	SANCUS_TCP_CONN_INPROGRESS,
	SANCUS_TCP_CONN_CONNECTED,
	SANCUS_TCP_CONN_RUNNING,
	SANCUS_TCP_CONN_FAILED,
};

struct sancus_tcp_conn_settings {
	void (*on_read) (struct sancus_tcp_conn *,
			 struct sancus_ev_loop *);
	void (*on_connect) (struct sancus_tcp_conn *,
			 struct sancus_ev_loop *);
	void (*on_error) (struct sancus_tcp_conn *,
			  struct sancus_ev_loop *,
			  enum sancus_tcp_conn_error);
};

struct sancus_tcp_conn {
	struct sancus_ev_fd io;

	enum sancus_tcp_conn_state state;
	struct timespec last_activity;

	const struct sancus_tcp_conn_settings *settings;
};

#define sancus_tcp_conn_fd(P)	((P)->io.fd)
#define sancus_tcp_conn_touch(C, L)	do { (C)->last_activity = sancus_ev_now(L); } while(0)
#define sancus_tcp_conn_elapsed(C, L)	sancus_time_elapsed(&(C)->last_activity, &sancus_ev_now(L))

/**
 * sancus_tcp_conn_start - start watching connection
 *
 * @self:	connection to be started
 * @loop:	event loop
 */
void sancus_tcp_conn_start(struct sancus_tcp_conn *self,
			   struct sancus_ev_loop *loop);

/**
 * sancus_tcp_conn_stop - stop watching connection
 *
 * @self:	connection to be stopped
 * @loop:	event loop
 */
void sancus_tcp_conn_stop(struct sancus_tcp_conn *self,
			  struct sancus_ev_loop *loop);

/**
 * sancus_tcp_conn_close - closes an already stopped connection
 *
 * @self:	connection to close
 */
void sancus_tcp_conn_close(struct sancus_tcp_conn *self);

/**
 * sancus_tcp_ipv4_connect - initialized and prepares an ipv4 tcp connection
 *
 * @self:	connection structure
 *
 * Returns 0 if @addr is invalid, 1 on success and -1 on error. errno set
 * accordingly.
 */
int sancus_tcp_ipv4_connect(struct sancus_tcp_conn *self,
			    const struct sancus_tcp_conn_settings *settings,
			    const char *addr, uint16_t port,
			    bool cloexec);

/**
 * sancus_tcp_ipv6_connect - initialized and prepares an ipv6 tcp connection
 *
 * @self:	connection structure
 *
 * Returns 0 if @addr is invalid, 1 on success and -1 on error. errno set
 * accordingly.
 */
int sancus_tcp_ipv6_connect(struct sancus_tcp_conn *self,
			    const struct sancus_tcp_conn_settings *settings,
			    const char *addr, uint16_t port,
			    bool cloexec);

/**
 * sancus_tcp_local_connect - initialized and prepares a local stream connection
 *
 * @self:	connection structure
 *
 * Returns 0 if @addr is invalid, 1 on success and -1 on error. errno set
 * accordingly.
 */
int sancus_tcp_local_connect(struct sancus_tcp_conn *self,
			     const struct sancus_tcp_conn_settings *settings,
			     const char *path, bool cloexec);

#endif
