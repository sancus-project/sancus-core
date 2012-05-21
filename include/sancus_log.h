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
#ifndef _SANCUS_LOG_H
#define _SANCUS_LOG_H

/**
 */
enum sancus_log_level {
	SANCUS_LOG_FATAL = 0,
	SANCUS_LOG_ALERT = 1,
	SANCUS_LOG_CRITICAL = 2,
	SANCUS_LOG_ERROR = 3,
	SANCUS_LOG_WARNING = 4,
	SANCUS_LOG_NOTICE = 5,
	SANCUS_LOG_INFO = 6,
	SANCUS_LOG_DEBUG = 7,
	SANCUS_LOG_TRACE = 8,
};

/**
 */
void sancus_log_write(enum sancus_log_level level, const char *name,
		      const char *dump, size_t dump_len,
		      const char *str);
/**
 */
void sancus_log_writef(enum sancus_log_level level, const char *name,
		       const char *dump, size_t dump_len,
		       const char *fmt, ...) TYPECHECK_PRINTF(5,6);

/**
 */
void sancus_log_trace(unsigned level, const char *name,
		      const char *filename, unsigned line, const char *func,
		      const char *str);
/**
 */
void sancus_log_tracef(unsigned level, const char *name,
		       const char *filename, unsigned line, const char *func,
		       const char *fmt, ...) TYPECHECK_PRINTF(6,7);

/*
 */
#define _log(L, N, S)		sancus_log_write(L, N, NULL, 0, S)
#define _plog(L, N, S)		sancus_log_writef(L, N, NULL, 0, S ": %s", strerror(errno))
#define _trace(L, N, S)		sancus_log_trace(L, N, __FILE__, __LINE__, __func__, S)
#define _logdump(...)		sancus_log_write(__VA_ARGS__)

#define _logf(L, N, F, ...)	sancus_log_writef(L, N, NULL, 0, F, __VA_ARGS__)
#define _plogf(L, N, F, ...)	sancus_log_writef(L, N, NULL, 0, F ": %s", __VA_ARGS__, strerror(errno))
#define _tracef(L, N, F, ...)	sancus_log_tracef(L, N, __FILE__, __LINE__, __func__, F, __VA_ARGS__)
#define _logdumpf(...)		sancus_log_writef(__VA_ARGS__)

#endif
