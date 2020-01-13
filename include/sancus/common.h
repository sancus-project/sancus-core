/*
 * Copyright (c) 2011-2013, Alejandro Mery <amery@geeks.cl>
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
#ifndef __SANCUS_COMMON_H__
#define __SANCUS_COMMON_H__

#include <stddef.h>
#include <sys/types.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/**
 * container_of - find reference to container of a given struct element
 */
#ifndef container_of
#define container_of(P,T,M)	((T *)((char *)(P) - offsetof(T, M)))
#endif

/** likely to be 1 */
#ifdef likely
#elif defined(HAVE___BUILTIN_EXPECT)
#	define likely(e)	__builtin_expect(!!(e), 1)
#else
#	define likely(e)	(e)
#endif

/** likely to be 0 */
#ifdef unlikely
#elif defined(HAVE___BUILTIN_EXPECT)
#	define unlikely(e)	__builtin_expect(!!(e), 0)
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
#elif defined(HAVE_VAR_ATTRIBUTE_UNUSED)
#	define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#else
#	define UNUSED(x) UNUSED_ ## x
#endif

/**
 * __attr_pure - function only depends on arguments
 */
#ifdef __attr_pure
#elif defined(HAVE_FUNC_ATTRIBUTE_PURE)
#	define __attr_pure __attribute__((pure))
#else
#	define __attr_pure
#endif

/**
 * __attr_const - function doesn't depend on anything
 */
#ifdef __attr_const
#elif defined(HAVE_FUNC_ATTRIBUTE_CONST)
#	define __attr_const __attribute__((const))
#else
#	define __attr_const
#endif

/**
 * __attr_noreturn - function doesn't return
 */
#ifdef __attr_noreturn
#elif defined(HAVE_FUNC_ATTRIBUTE_NORETURN)
#	define __attr_noreturn __attribute__((noreturn))
#else
#	define __attr_noreturn
#endif

/**
 * __attr_printf2 - Check printf format
 */
#ifdef __attr_printf2
#elif defined(HAVE_FUNC_ATTRIBUTE_FORMAT)
#	define __attr_printf2(I, J)	__attribute__((format (printf, I, J)))
#else
#	define __attr_printf2(I, J)
#endif

/**
 * __attr_printf - Shortcut to check printf format
 */
#ifndef __attr_printf
#	define __attr_printf(N) __attr_printf2(N, N+1)
#endif

/**
 * __attr_vprintf - Check printf format, but without varadic
 */
#ifdef __attr_vprintf
#elif defined(HAVE_FUNC_ATTRIBUTE_FORMAT)
#	define __attr_vprintf(I)	__attribute__((format (printf, I, 0)))
#else
#	define __attr_vprintf(I)
#endif

/**
 * __attr_scanf2 - Check scanf format
 */
#ifdef __attr_scanf2
#elif defined(HAVE_FUNC_ATTRIBUTE_FORMAT)
#	define __attr_scanf2(I, J)	__attribute__((format (scanf, I, J)))
#else
#	define __attr_scanf2(I, J)
#endif

/**
 * __attr_scanf - Shortcut to check scanf format
 */
#ifndef __attr_scanf
#	define __attr_scanf(N) __attr_scanf2(N, N+1)
#endif

#endif /* !__SANCUS_COMMON_H__ */
