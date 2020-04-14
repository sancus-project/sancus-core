#ifndef __SANCUS_STRING_H__
#define __SANCUS_STRING_H__

#include <string.h>

static inline ssize_t sancus_memcpy(char *dest, ssize_t dest_size, const char *src, size_t n)
{
	ssize_t ret;

	if ((ssize_t)n > dest_size) {
		n = (size_t)dest_size;
		ret = -ENOBUFS;
	} else {
		ret = (ssize_t)n;
	}

	if (n > 0)
		memcpy(dest, src, n);
	return ret;
}

static inline ssize_t sancus_strncpy(char *dest, ssize_t dest_size, const char *src, size_t n)
{
	ssize_t ret = -ENOBUFS;

	if (dest_size > 0) {
		size_t sz = (size_t)dest_size;
		if (n < sz)
			ret = (ssize_t)n;
		else
			n = sz - 1;

		if (n > 0)
			memcpy(dest, src, n);

		dest[n] = '\0';
	}

	return ret;
}

static inline ssize_t sancus_strcpy(char *dest, ssize_t dest_size, const char *src)
{
	size_t n;

	if (src != NULL && *src != '\0')
		n = strlen(src);
	else
		n = 0;

	return sancus_strncpy(dest, dest_size, src, n);
}

#endif
