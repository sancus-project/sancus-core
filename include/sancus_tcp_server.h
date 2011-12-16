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

struct sacus_tcp_server;

/**
 * struct sancus_tcp_server_settings - driving callbacks of tcp server
 * @pre_bind:	hook to tweak fd's sockopts before calling bind()
 */
struct sancus_tcp_server_settings {
	void (*pre_bind) (struct sacus_tcp_server *);
};

/**
 * struct sacus_tcp_server - tcp server
 *
 * @fd:		listening file descriptor
 * @settings:	driving callbacks
 */
struct sacus_tcp_server {
	int fd;
	struct sancus_tcp_server_settings *settings;
};

/**
 * sancus_tcp_ipv4_listen - initializes and prepares ipv4 tcp server
 */
int sancus_tcp_ipv4_listen(struct sacus_tcp_server *self,
			   struct sancus_tcp_server_settings *settings,
			   const char *addr, unsigned port,
			   bool cloexec, unsigned backlog);

/**
 * sancus_tcp_ipv6_listen - initializes and prepares ipv6 tcp server
 */
int sancus_tcp_ipv6_listen(struct sacus_tcp_server *self,
			   struct sancus_tcp_server_settings *settings,
			   const char *addr, unsigned port,
			   bool cloexec, unsigned backlog);

#endif /* !_SANCUS_TCP_SERVER_H */
