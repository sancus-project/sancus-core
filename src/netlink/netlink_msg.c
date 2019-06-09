/*
 * Copyright (c) 2012, Christian Wiese <chris@opensde.org>
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

#include <sancus/common.h>
#include <sancus/ev.h>

#include <stdint.h>		/* for uint16_t */
#include <stdbool.h>
#include <string.h>		/* for memset */

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include <sancus/netlink.h>

/**
 * Netlink Message handling
 */

bool sancus_nl_msg_ok(const struct nlmsghdr *nlh, int len)
{
	return len >= (int)sizeof(struct nlmsghdr) &&
	       nlh->nlmsg_len >= sizeof(struct nlmsghdr) &&
	       (int)nlh->nlmsg_len <= len;
}

struct nlmsghdr *sancus_nl_msg_next(const struct nlmsghdr *nlh, int *len)
{
	*len -= SANCUS_NL_ALIGN(nlh->nlmsg_len);
	return (struct nlmsghdr *)((struct nlmsghdr *)nlh + SANCUS_NL_ALIGN(nlh->nlmsg_len));
}

bool sancus_nl_msg_portid_ok(const struct nlmsghdr *nlh, unsigned int portid)
{
	return nlh->nlmsg_pid && portid ? nlh->nlmsg_pid == portid : true;
}

void *sancus_nl_msg_get_payload(const struct nlmsghdr *nlh)
{
	return (char *)nlh + SANCUS_NL_MSG_HDRLEN;
}

void *sancus_nl_msg_get_payload_offset(const struct nlmsghdr *nlh, size_t offset)
{
	return (char *)nlh + SANCUS_NL_MSG_HDRLEN + SANCUS_NL_ALIGN(offset);
}

void *sancus_nl_msg_get_payload_tail(const struct nlmsghdr *nlh)
{
        return (char *)nlh + SANCUS_NL_ALIGN(nlh->nlmsg_len);
}

struct nlmsghdr *sancus_nl_msg_put_header(void *buf)
{
	int len = SANCUS_NL_ALIGN(sizeof(struct nlmsghdr));
	struct nlmsghdr *nlh = buf;

	/* zero'ing the memory occupied by the netlink header */
	memset(buf, 0, len);
	/* initialise nlmsg_len field */
	nlh->nlmsg_len = len;

	return nlh;
}

void *sancus_nl_msg_put_extra_header(struct nlmsghdr *nlh, size_t size)
{
	char *ptr = (char *)nlh + nlh->nlmsg_len;
	size_t len = SANCUS_NL_ALIGN(size);

	/* zero'ing the memory occupied by the extra header */
	memset(ptr, 0, len);
	/* increase the nlmsg_len field by the lenght of the extra header */
	nlh->nlmsg_len += len;

	return ptr;
}
