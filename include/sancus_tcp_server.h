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
#ifndef _SANCUS_TCP_SERVER_H
#define _SANCUS_TCP_SERVER_H

struct sancus_tcp_server;

/**
 * enum sancus_tcp_server_error - list of possible errors
 *
 * @SANCUS_TCP_SERVER_WATCHER_ERROR:	connect watcher error, server has been closed
 * @SANCUS_TCP_SERVER_ACCEPT_ERROR:	accept() call failed, check %errno
 */
enum sancus_tcp_server_error {
	SANCUS_TCP_SERVER_WATCHER_ERROR,
	SANCUS_TCP_SERVER_ACCEPT_ERROR,
};

/**
 * struct sancus_tcp_server_settings - driving callbacks of tcp server
 *
 * @pre_bind:	hook to tweak fd's sockopts before calling bind()
 * @on_connect:	new connection received, return %false if it should be closed
 * @on_error:	an error has happened, tell the world
 */
struct sancus_tcp_server_settings {
	void (*pre_bind) (struct sancus_tcp_server *);

	bool (*on_connect) (struct sancus_tcp_server *, struct ev_loop *,
			    int, struct sockaddr *, socklen_t);

	void (*on_error) (struct sancus_tcp_server *,
			  struct ev_loop *,
			  enum sancus_tcp_server_error);
};

/**
 * struct sancus_tcp_server - tcp server
 *
 * @connect:	connection watcher
 * @settings:	driving callbacks
 */
struct sancus_tcp_server {
	struct ev_io connect;

	struct sancus_tcp_server_settings *settings;
};

/**
 * sancus_tcp_server_start - start watching port
 *
 * @self:	server to be started
 * @loop:	event loop
 *
 * The server shall not be already active. Nothing returned
 */
void sancus_tcp_server_start(struct sancus_tcp_server *self, struct ev_loop *loop);

/**
 * sancus_tcp_server_stop - stop watching port
 * @self:	server to be stopped
 * @loop:	event loop
 *
 * The server shall not be already stopped. Nothing returned
 */
void sancus_tcp_server_stop(struct sancus_tcp_server *self, struct ev_loop *loop);

/**
 * sancus_tcp_server_close - closes an already stopped port
 *
 * @self:	server to close
 */
void sancus_tcp_server_close(struct sancus_tcp_server *self);

/**
 * sancus_tcp_ipv4_listen - initializes and prepares ipv4 tcp server
 *
 * @self:	server structure to initialize
 * @settings:	driving callbacks
 * @addr:	ipv4 string
 * @port:	port number
 * @cloexec:	enable close-on-exec or not
 * @backlog:	backlog value for listen()
 *
 * Returns 0 if @addr is invalid, 1 on success and -1 on error. errno set
 * accordingly.
 */
int sancus_tcp_ipv4_listen(struct sancus_tcp_server *self,
			   struct sancus_tcp_server_settings *settings,
			   const char *addr, unsigned port,
			   bool cloexec, unsigned backlog);

/**
 * sancus_tcp_ipv6_listen - initializes and prepares ipv6 tcp server
 *
 * @self:	server structure to initialize
 * @settings:	driving callbacks
 * @addr:	ipv6 string
 * @port:	port number
 * @cloexec:	enable close-on-exec or not
 * @backlog:	backlog value for listen()
 *
 * Returns 0 if @addr is invalid, 1 on success and -1 on error. errno set
 * accordingly.
 */
int sancus_tcp_ipv6_listen(struct sancus_tcp_server *self,
			   struct sancus_tcp_server_settings *settings,
			   const char *addr, unsigned port,
			   bool cloexec, unsigned backlog);

/**
 * sancus_tcp_local_listen - initializes and prepares local domain tcp server
 *
 * @self:	server structure to initialize
 * @settings:	driving callbacks
 * @path:	location for the unix/local domain socket
 * @cloexec:	enable close-on-exec or not
 * @backlog:	backlog value for listen()
 *
 * Returns 0 if @addr is invalid, 1 on success and -1 on error. errno set
 * accordingly.
 */
int sancus_tcp_local_listen(struct sancus_tcp_server *self,
			    struct sancus_tcp_server_settings *settings,
			    const char *path,
			    bool cloexec, unsigned backlog);

/**
 * sancus_tcp_server_fd - returns fd been listened by a tcp server
 *
 * @S:	server structure to query
 */
#define sancus_tcp_server_fd(S)		(S)->connect.fd

/**
 * sancus_tcp_server_is_active - checks if a server has not been closed nor stoped
 *
 * @S:	server structure to query
 */
#define sancus_tcp_server_is_active(S)	((S)->connect.fd > 0 && ev_is_active(&(S)->connect))

#endif /* !_SANCUS_TCP_SERVER_H */
