#ifndef __SANCUS_STRING_H__
#define __SANCUS_STRING_H__

#include <string.h>

static inline ssize_t sancus_memcpy(char *dest, ssize_t dest_size, const char *src, size_t n)
{
	ssize_t ret;

	if ((ssize_t)n > dest_size) {
		n = dest_size;
		ret = -ENOBUFS;
	} else {
		ret = n;
	}

	if (n > 0)
		memcpy(dest, src, n);
	return ret;
}

#endif
