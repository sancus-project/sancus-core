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
#ifndef _SANCUS_FD_H
#define _SANCUS_FD_H

#ifdef _FCNTL_H
/**
 * sancus_open - auto-retrying wrapper for open(2) with easier cloexec support
 */
static inline int sancus_open(const char *pathname, int flags, int cloexec, mode_t mode)
{
	int fd;
#ifdef O_CLOEXEC
	if (cloexec)
		flags |= O_CLOEXEC;
	else
		flags &= ~O_CLOEXEC;
#endif
open_retry:
	if ((fd = open(pathname, flags, mode)) < 0) {
		if (errno == EINTR)
			goto open_retry;
		else
			goto open_done;
	}

	if (cloexec) {
		int fl = fcntl(fd, F_GETFL);
		if (fl < 0)
			goto open_failed;
#ifdef O_CLOEXEC
		if ((fl & FD_CLOEXEC) == 0)
#endif
			if (fcntl(fd, F_SETFL, fl|FD_CLOEXEC) < 0) {
				goto open_failed;
			}
	}
	goto open_done;
open_failed:
	close(fd);
	fd = -1;
open_done:
	return fd;
}
#endif

#ifdef _UNISTD_H
/**
 * sancus_close - auto-retrying wrapper for close(2) which resets the fd on success
 */
static inline int sancus_close(int *fd)
{
	int ret = 0;
	if (fd != NULL && *fd >= 0) {
close_retry:
		ret = close(*fd);
		if (ret == 0)
			*fd = -0xdead;
		else if (errno == EINTR)
			goto close_retry;
	}
	return ret;
}

/**
 * sancus_read - auto-retrying wrapper for read(2)
 */
static inline ssize_t sancus_read(int fd, char *buf, size_t count)
{
	ssize_t rc;
read_retry:
	rc = read(fd, buf, count);
	if (rc < 0 && errno == EINTR)
		goto read_retry;
	return rc;
}

/**
 * sancus_write - auto-retrying wrapper for write(2)
 */
static inline ssize_t sancus_write(int fd, const char *data, size_t size)
{
	int wc, wt = 0;

	while (size > 0) {
write_retry:
		wc = write(fd, data, size);
		if (wc > 0) {
			wt += wc;
			size -= wc;
			data += wc;
		} else if (wc < 0) {
			if (errno == EINTR)
				goto write_retry;
			return wc;
		}
	}

	return wt;
}
#endif

#ifdef _SYS_UIO_H
/**
 * sancus_writev - auto-retrying wrapper for writev(2)
 */
static inline ssize_t sancus_writev(int fd, struct iovec *iov, int iovcnt)
{
	int wt = 0;

	while (iovcnt) {
		register int wc;
try_write:
		wc = writev(fd, iov, iovcnt);
		if (wc > 0) {
			wt += wc;

			/* consume accordingly */
			while (wc) {
				if ((unsigned)wc >= iov->iov_len) {
					wc -= iov->iov_len;
					iovcnt--;
					iov++;
				} else {
					iov->iov_len -= wc;
					/* GCC: warning: pointer of type ‘void *’ used in arithmetic */
					iov->iov_base = (char*)iov->iov_base+wc;
					wc = 0;
				}
			}
		} else if (wc < 0) {
			if (errno == EINTR)
				goto try_write;
			return wc;
		}
	}

	return wt;
}
#endif /* _SYS_UIO_H */

#endif /* !_SANCUS_FD_H */
