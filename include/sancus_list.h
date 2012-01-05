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
 * sancus_list_insert - inserts element at the begining of a list
 */
#define sancus_list_insert(H, E)	sancus_list_inject((E), (H), (H)->next)

/**
 * sancus_list_append - append element at the end of a list
 */
#define sancus_list_append(H, E)	sancus_list_inject((E), (H)->prev, (H))

/**
 * sancus_list_isempty - tells if there are no (other) elements in this list
 */
#define sancus_list_isempty(H)		((H)->next == (H))
#endif /* !_SANCUS_LIST_H */
