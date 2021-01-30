#include <sancus/common.h>
#include <sancus/fd.h>

#include <sys/uio.h>

static inline int sancus__openat(int dirfd, const char *pathname, int flags, int cloexec, mode_t mode)

{
	int e, fd;

#ifdef O_CLOEXEC
	if (cloexec)
		flags |= O_CLOEXEC;
	else
		flags &= ~O_CLOEXEC;
#endif
open_retry:
	if ((fd = openat(dirfd, pathname, flags, mode)) < 0) {
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
	e = errno;
	sancus_close(fd);
	errno = e;
	fd = -e;
open_done:
	return fd;
}

int sancus_openat(int dirfd, const char *pathname, int flags, int cloexec, mode_t mode)
{
	return sancus__openat(dirfd, pathname, flags, cloexec, mode);
}

int sancus_open(const char *pathname, int flags, int cloexec, mode_t mode)
{
	return sancus__openat(AT_FDCWD, pathname, flags, cloexec, mode);
}

ssize_t sancus_writev(int fd, struct iovec *iov, int iovcnt)
{
	ssize_t wt = 0;

	if (iovcnt < 0)
		return -EINVAL;

	while (iovcnt) {
		ssize_t wc = writev(fd, iov, iovcnt);

		if (wc > 0) {
			size_t n = (size_t)wc;
			wt += wc;

			/* consume accordingly */
			while (n) {
				if (n >= iov->iov_len) {
					n -= iov->iov_len;
					iovcnt--;
					iov++;
				} else {
					iov->iov_len -= n;
					/* GCC: warning: pointer of type ‘void *’ used in arithmetic */
					iov->iov_base = (char*)iov->iov_base+wc;
					n = 0;
				}
			}
		} else if (wc < 0 && errno != EINTR) {
			return -errno;
		}
	}

	return wt;
}
