/*
 * Copyright (c) 2010-2012, Alejandro Mery <amery@geeks.cl>
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
#ifndef _SANCUS_ALLOC_H
#define _SANCUS_ALLOC_H

extern void *(*_sancus_realloc) (void *, size_t);

/**
 */
#define sancus_alloc(S) _sancus_realloc(NULL, S)

/**
 */
#define sancus_realloc(P, S) _sancus_realloc(P, S)

/**
 */
static inline void *sancus_zalloc(size_t size)
{
	void *ptr = sancus_alloc(size);
	if (ptr)
		memset(ptr, 0x00, size);
	return ptr;
}

/**
 */
#define sancus_free(P)	do { _sancus_realloc(P, 0); (P) = NULL; } while(0)

/**
 */
static inline void sancus_set_realloc(void *(*f) (void *, size_t))
{
	_sancus_realloc = f;
}

#endif /* !_SANCUS_ALLOC_H */
