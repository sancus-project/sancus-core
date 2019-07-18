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
#ifndef __SANCUS_FD_H__
#define __SANCUS_FD_H__

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

struct iovec;

/**
 * sancus_openat - auto-retrying wrapper for openat(2) with easier cloexec support
 */
int sancus_openat(int dirfd, const char *pathname, int flags, int cloexec, mode_t mode);

/**
 * sancus_open - auto-retrying wrapper for open(2) with easier cloexec support
 */
int sancus_open(const char *pathname, int flags, int cloexec, mode_t mode);

/**
 * sancus_close - auto-retrying wrapper for close(2)
 */
static inline int sancus_close(int fd)
{
	while (1) {
		if (close(fd) == 0)
			return 0;
		else if (errno != EINTR)
			return -errno;
	}
}

static inline int sancus_close2(int *fd)
{
	int rc = 0;
	if (fd != NULL) {
		rc = sancus_close(*fd);
		*fd = -1;
	}
	return rc;
}

/**
 * sancus_read - auto-retrying wrapper for read(2)
 */
static inline ssize_t sancus_read(int fd, char *buf, size_t count)
{
	while (1) {
		ssize_t rc = read(fd, buf, count);
		if (rc >= 0)
			return rc;
		else if (errno != EINTR)
			return -errno;
	}
}

/**
 * sancus_write - auto-retrying wrapper for write(2)
 */
static inline ssize_t sancus_write(int fd, const char *data, size_t count)
{
	ssize_t wt = 0;

	while (count) {
		ssize_t wc = write(fd, data, count);

		if (wc > 0) {
			count -= wc;
			data += wc;
			wt += wc;
		} else if (wc < 0 && errno != EINTR) {
			return -errno;
		}
	}

	return wt;
}

/**
 * sancus_writev - auto-retrying wrapper for writev(2)
 */
ssize_t sancus_writev(int fd, struct iovec *iov, size_t iovcnt);

#endif /* !__SANCUS_FD_H__ */
