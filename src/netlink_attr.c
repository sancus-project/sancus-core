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

#include <stdbool.h>
#include <stdint.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include "sancus_common.h"
#include "sancus_netlink.h"

/**
 * Netlink Attribute handling
 */

uint16_t sancus_nl_attr_get_type(const struct nlattr *attr)
{
	return attr->nla_type & NLA_TYPE_MASK;
}

uint16_t sancus_nl_attr_get_len(const struct nlattr *attr)
{
	return attr->nla_len;
}

uint16_t sancus_nl_attr_get_payload_len(const struct nlattr *attr)
{
	return attr->nla_len - SANCUS_NL_ATTR_HDRLEN;
}

void *sancus_nl_attr_get_payload(const struct nlattr *attr)
{
	return (char *)attr + SANCUS_NL_ATTR_HDRLEN;
}

bool sancus_nl_attr_ok(const struct nlattr *attr, int len)
{
	return len >= (int)sizeof(struct nlattr) &&
	       attr->nla_len >= sizeof(struct nlattr) &&
	       (int)attr->nla_len <= len;
}

struct nlattr *sancus_nl_attr_next(const struct nlattr *attr)
{
	return (struct nlattr *)((char *)attr + SANCUS_NL_ALIGN(attr->nla_len));
}

int sancus_nl_attr_type_valid(const struct nlattr *attr, uint16_t max)
{
	if (sancus_nl_attr_get_type(attr) > max) {
		errno = EOPNOTSUPP;
		return -1;
	}
	return 1;
}

int sancus_nl_attr_parse(const struct nlmsghdr *nlh, unsigned int offset,
			 sancus_nl_attr_parse_cb cb, void *data)
{
	int ret = SANCUS_NL_CB_OK;
	const struct nlattr *attr;

	sancus_nl_attr_foreach(attr, nlh, offset)
		if ((ret = cb(attr, data)) <= SANCUS_NL_CB_STOP)
			return ret;
	return ret;
}

uint8_t sancus_nl_attr_get_u8(const struct nlattr *attr)
{
	return *((uint8_t *)sancus_nl_attr_get_payload(attr));
}

uint16_t sancus_nl_attr_get_u16(const struct nlattr *attr)
{
	return *((uint16_t *)sancus_nl_attr_get_payload(attr));
}

uint32_t sancus_nl_attr_get_u32(const struct nlattr *attr)
{
	return *((uint32_t *)sancus_nl_attr_get_payload(attr));
}

uint64_t sancus_nl_attr_get_u64(const struct nlattr *attr)
{
	return *((uint64_t *)sancus_nl_attr_get_payload(attr));
}

const char *sancus_nl_attr_get_str(const struct nlattr *attr)
{
	return sancus_nl_attr_get_payload(attr);
}
