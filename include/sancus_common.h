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
#ifndef _SANCUS_COMMON_H
#define _SANCUS_COMMON_H

#include <ev.h>

/*
 * helpers to avoid strict-aliasing problems in ev.h
 */
#undef ev_io_init
static inline void ev_io_init(struct ev_io *w,
			      void (*cb) (struct ev_loop *, struct ev_io *, int),
			      int fd, int events)
{
	ev_init(w, cb);
	ev_io_set(w, fd, events);
}

#undef ev_is_active
#define ev_is_active(w)	(w)->active

/**
 * container_of - find reference to container of a given struct element
 */
#ifndef container_of
#define container_of(P,T,M)	(T *)((char *)(P) - offsetof(T, M))
#endif

/** likely to be 1 */
#ifdef likely
#elif defined(__GNUC__)
#	define likely(e)	__builtin_expect((e), 1)
#else
#	define likely(e)	(e)
#endif

/** likely to be 0 */
#ifdef unlikely
#elif defined(__GNUC__)
#	define unlikely(e)	__builtin_expect((e), 0)
#else
#	define unlikely(e)	(e)
#endif

/**
 * ARRAY_SIZE - Number of elements of an array
 */
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(A)	(sizeof(A)/sizeof((A)[0]))
#endif

/**
 * UNUSED - Not used function argument
 */
#ifdef UNUSED
#elif defined(__GNUC__)
#	define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#else
#	define UNUSED(x) x
#endif

/**
 * TYPECHECK_PRINTF - Check printf format
 */
#ifdef TYPECHECK_PRINTF
#elif defined(__GNUC__)
#	define TYPECHECK_PRINTF(I, J)	__attribute__((format (printf, I, J)))
#else
#	define TYPECHECK_PRINTF(I, J)
#endif

#endif /* !_SANCUS_COMMON_H */
