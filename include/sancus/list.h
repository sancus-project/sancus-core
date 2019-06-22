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
#ifndef __SANCUS_LIST_H__
#define __SANCUS_LIST_H__

/**
 * struct sancus_list - double linked list element
 */
struct sancus_list {
	struct sancus_list *prev;
	struct sancus_list *next;
};

#define DECL_SANCUS_LIST(N) struct sancus_list N = { &N, &N }

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
static inline void sancus_list__del(struct sancus_list *prev,
				    struct sancus_list *next)
{
	next->prev = prev;
	prev->next = next;
}
#define sancus_list_del(S)	sancus_list__del((S)->prev, (S)->next)

/**
 * sancus_list_swap - swaps two elements in a list
 */
static inline void sancus_list_swap(struct sancus_list *a, struct sancus_list *b)
{
	struct sancus_list aux = { a->prev, a->next };

	*a = (struct sancus_list) { b->prev, b->next };
	*b = (struct sancus_list) { aux.prev, aux.next };
}


/**
 * sancus_list_foreach - iterates over a list
 */
#define sancus_list_foreach(H, I)	for (struct sancus_list *I = (H)->next; (I) != (H); (I) = (I)->next)

/**
 * sancus_list_foreach2 - safely iterate over a list
 */
#define sancus_list_foreach2(H, I, N)	for (struct sancus_list *I = (H)->next, *N = (I)->next; (I) != (H); (I) = (N), (N) = (I)->next)

/**
 * sancus_list_foreach_back - iterates over a list backward
 */
#define sancus_list_foreach_back(H, I)	for (struct sancus_list *I = (H)->prev; (I) != (H); (I) = (I)->prev)

/**
 * sancus_list_foreach_back2 - safely iterate over a list backward
 */
#define sancus_list_foreach_back2(H, I, P)	for (struct sancus_list *I = (H)->prev, *P = (I)->prev; (I) != (H); (I) = (P), (P) = (I)->prev)

/**
 * sancus_list_get gets a given element on a list, backward or forward.
 */
static inline struct sancus_list *sancus_list_get(struct sancus_list *head, ssize_t n)
{
	if (n < 0) {
		sancus_list_foreach_back(head, item) {
			if (++n == 0)
				return item;
		}
	} else {
		sancus_list_foreach(head, item) {
			if (n-- == 0)
				return item;
		}
	}
	return NULL;
}

/**
 * sancus_list_is_empty - tells if there are no (other) elements in this list
 */
#define sancus_list_is_empty(H)		((H)->next == (H))

/**
 * sancus_list_first - returns the first element on a list, if any
 */
#define sancus_list_first(H)		(sancus_list_is_empty(H) ? NULL : (H)->next)

/**
 * sancus_list_last - returns the last element of a list, if any
 */
#define sancus_list_last(H)		(sancus_list_is_empty(H) ? NULL : (H)->prev)

/**
 * sancus_list_next - returns the next elemnet on a list, if any
 */
#define sancus_list_next(H, E)		(((H) == NULL || (E) == NULL || (E)->next == (H)) ?  NULL : (E)->next)

#endif /* !__SANCUS_LIST_H__ */
