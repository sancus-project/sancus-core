/*
 * Copyright (c) 2011-2012, Alejandro Mery <amery@geeks.cl>
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

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>

#include <sys/uio.h>	/* writev() */

#include <sancus/common.h>
#include <sancus/fmt.h>
#include <sancus/fd.h>
#include <sancus/log.h>

/* arbitrary sizes */
#define STR_BUFSIZE 1024
#define TRACE_STR_BUFSIZE 1024

/*
 */
static ssize_t _write_stderr(enum sancus_log_level level, const char *name,
			     const char *trace, size_t trace_len,
			     const char *dump, size_t dump_len,
			     const char *str, size_t str_len)
{
	char buf[1024]; /* arbitrary */
	struct iovec v[8];
	size_t l=0, l2;
	char level_s[] = "<0> ";

	assert(level < 10);

	/* "<?> " */
	level_s[1] += level;
	v[l++] = (struct iovec) { level_s, 4 };

	/* "name: " */
	l2 = name ? strlen(name) : 0;
	if (l2 > 0) {
		v[l++] = (struct iovec) { (void*)name, l2 };
		if ((str_len + trace_len) > 0)
			v[l++] = (struct iovec) { ": ", 2 };
	}

	/* "trace" */
	if (trace_len > 0)
		v[l++] = (struct iovec) { (void*)trace, trace_len };

	/* "str" */
	if (str_len > 0)
		v[l++] = (struct iovec) { (void*)str, str_len };

	if (dump_len > 0) {
		/* ": \"...\" (n)\n" */
		char *p = buf;
		size_t pl = sizeof(buf);

		v[l++] = (struct iovec) { ": \"", 3 };

		l2 = sancus_fmt_ascii_cstr(p, pl, dump, dump_len);
		v[l++] = (struct iovec) { p, l2 };
		p += l2;
		pl -= l2;

		l2 = snprintf(p, pl, "\" (%zu)\n", dump_len);
		v[l++] = (struct iovec) { p, l2 };
	} else {
		/* "\n" */
		v[l++] = (struct iovec) { "\n", 1 };
	}

	return sancus_writev(2, v, l);
}

static inline size_t _fmt(char *buf, size_t size, const char *fmt, va_list ap)
{
	size_t l = vsnprintf(buf, size, fmt, ap);

	/* truncated? */
	if (l > size-1) {
		l = size-1;
		buf[l] = '\0';
	}

	return l;
}

static inline size_t _fmt_trace(char *buf, size_t size,
				const char *filename, unsigned line, const char *func)
{
	if (filename)
		return snprintf(buf, size, "%s:%u: %s: ", filename, line, func);
	else
		return snprintf(buf, size, "%s: ", func);
}

/*
 * exported
 */
void sancus_log_write(enum sancus_log_level level, const char *name,
		      const char *dump, size_t dump_len,
		      const char *str)
{
	size_t str_len = str ? strlen(str) : 0;

	_write_stderr(level, name, NULL, 0,
		      dump, dump_len, str, str_len);
}


void sancus_log_writef(enum sancus_log_level level, const char *name,
		       const char *dump, size_t dump_len,
		       const char *fmt, ...)
{
	va_list ap;
	char str[STR_BUFSIZE];
	size_t str_len;

	va_start(ap, fmt);
	str_len = _fmt(str, sizeof(str), fmt, ap);
	va_end(ap);

	_write_stderr(level, name, NULL, 0,
		      dump, dump_len, str, str_len);
}

void sancus_log_trace(unsigned level, const char *name,
		      const char *filename, unsigned line, const char *func,
		      const char *str)
{
	char trace[TRACE_STR_BUFSIZE];
	size_t trace_len = _fmt_trace(trace, sizeof(trace), filename, line, func);
	size_t str_len = str ? strlen(str) : 0;

	_write_stderr(level, name, trace, trace_len,
		      NULL, 0, str, str_len);
}

void sancus_log_tracef(unsigned level, const char *name,
		       const char *filename, unsigned line, const char *func,
		       const char *fmt, ...)
{
	va_list ap;
	char str[STR_BUFSIZE];
	size_t str_len;

	char trace[TRACE_STR_BUFSIZE];
	size_t trace_len = _fmt_trace(trace, sizeof(trace), filename, line, func);

	va_start(ap, fmt);
	str_len = _fmt(str, sizeof(str), fmt, ap);
	va_end(ap);

	_write_stderr(level, name, trace, trace_len,
		      NULL, 0, str, str_len);
}
