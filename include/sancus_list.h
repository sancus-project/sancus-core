/*
 * Copyright (c) 2012, Alejandro Mery <amery@geeks.cl>
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
#ifndef _SANCUS_LIST_H
#define _SANCUS_LIST_H

/**
 * struct sancus_list - double linked list element
 */
struct sancus_list {
	struct sancus_list *prev;
	struct sancus_list *next;
};

/**
 * sancus_list_init - initializes list element
 */
static inline void sancus_list_init(struct sancus_list *self)
{
	*self = (struct sancus_list) { self, self };
}

/**
 * sancus_list_inject - injects a list element between two other
 */
static inline void sancus_list_inject(struct sancus_list *self,
				      struct sancus_list *prev,
				      struct sancus_list *next)
{
	*self = (struct sancus_list) { prev, next };

	next->prev = prev->next = self;
}

/**
 * sancus_list_insert - inserts item at the begining of a list
 */
#define sancus_list_insert(H, I)	sancus_list_inject((I), (H), (H)->next)

/**
 * sancus_list_append - append item at the end of a list
 */
#define sancus_list_append(H, I)	sancus_list_inject((I), (H)->prev, (H))

/**
 * sancus_list_del - removes item from the list
 */
static inline void __sancus_list_del(struct sancus_list *prev,
				     struct sancus_list *next)
{
	next->prev = prev;
	prev->next = next;
}
#define sancus_list_del(S)	__sancus_list_del((S)->prev, (S)->next)

/**
 * sancus_list_foreach - iterates over a list
 */
#define sancus_list_foreach(H, I)	for(struct sancus_list *I = (H)->next; (I) != (H); (I) = (I)->next)

/**
 * sancus_list_foreach2 - safely iterate over a list
 */
#define sancus_list_foreach2(H, I, N)	for(struct sancus_list *I = (H)->next, *N = (I)->next; (I) != (H); (I) = (N), (N) = (I)->next)

/**
 * sancus_list_isempty - tells if there are no (other) elements in this list
 */
#define sancus_list_isempty(H)		((H)->next == (H))

/**
 * sancus_list_first - returns the first element on a list, if any
 */
#define sancus_list_first(H)		(sancus_list_isempty(H)?NULL:(H)->next)

#define sancus_list_next(H, E)		((E)->next == (H) ? NULL : (E)->next)

#endif /* !_SANCUS_LIST_H */
