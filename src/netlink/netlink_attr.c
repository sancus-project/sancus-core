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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>		/* for memcpy, strlen */
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include <sancus/netlink.h>
#include <sancus/types.h>

/**
 * Netlink Attribute handling
 */

uint16_t sancus_nl_attr_get_type(const struct nlattr *attr)
{
	return attr->nla_type & (uint16_t)NLA_TYPE_MASK;
}

uint16_t sancus_nl_attr_get_len(const struct nlattr *attr)
{
	return attr->nla_len;
}

uint16_t sancus_nl_attr_get_payload_len(const struct nlattr *attr)
{
	return (uint16_t)(attr->nla_len - SANCUS_NL_ATTR_HDRLEN);
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

/* minimum length of basic attribute data types */
static const size_t sancus_nl_attr_data_type_minlen[SANCUS_NL_ATTR_TYPE_MAX + 1] = {
	[SANCUS_NL_ATTR_TYPE_U8]		= sizeof(uint8_t),
	[SANCUS_NL_ATTR_TYPE_U16]		= sizeof(uint16_t),
	[SANCUS_NL_ATTR_TYPE_U32]		= sizeof(uint32_t),
	[SANCUS_NL_ATTR_TYPE_U64]		= sizeof(uint64_t),
	[SANCUS_NL_ATTR_TYPE_STRING]		= 1,
	[SANCUS_NL_ATTR_TYPE_NUL_STRING]	= 1,
};

/* internally used generic attribute validation */
static int validate_attr(const struct nlattr *attr,
			 enum sancus_nl_attr_data_type type,
			 size_t expected_len)
{
	uint16_t len = sancus_nl_attr_get_payload_len(attr);
	const char *payload = sancus_nl_attr_get_payload(attr);

	if (len < expected_len) {
		errno = ERANGE;
		return -1;
	}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
	switch(type) {
	case SANCUS_NL_ATTR_TYPE_FLAG:
		/* len is unused */
		if (len > 0) {
			errno = ERANGE;
			return -1;
		}
		break;
	case SANCUS_NL_ATTR_TYPE_STRING:
		if (len == 0) {
			errno = ERANGE;
			return -1;
		}
		break;
	case SANCUS_NL_ATTR_TYPE_NUL_STRING:
		if (len == 0) {
			errno = ERANGE;
			return -1;
		}
		/* payload needs to be NUL terminated */
		if (payload[len-1] != '\0') {
			errno = EINVAL;
			return -1;
		}
		break;
	case SANCUS_NL_ATTR_TYPE_NESTED:
		/* empty nested attributes are allowed */
		if (len == 0)
			break;
		/* if not empty, it must contain one header */
		if (len < SANCUS_NL_ATTR_HDRLEN) {
			errno = ERANGE;
			return -1;
		}
		break;
	default: /* gcc -Wswitch warns about unhandled cases otherwise */
		break;
	}
#pragma GCC diagnostic pop

	return 0;
}

int sancus_nl_attr_validate_minlen(const struct nlattr *attr,
				   enum sancus_nl_attr_data_type type)
{
	size_t minlen;

	if (type >= SANCUS_NL_ATTR_TYPE_MAX) {
		errno = EINVAL;
		return -1;
	}
	minlen = sancus_nl_attr_data_type_minlen[type];
	return validate_attr(attr, type, minlen);
}

int sancus_nl_attr_validate_explen(const struct nlattr *attr,
				   enum sancus_nl_attr_data_type type,
				   size_t explen)
{
	if (type >= SANCUS_NL_ATTR_TYPE_MAX) {
		errno = EINVAL;
		return -1;
	}
	return validate_attr(attr, type, explen);
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

const char *sancus_nl_attr_get_string(const struct nlattr *attr)
{
	return sancus_nl_attr_get_payload(attr);
}

/**
 * __sancus_nl_attr_put_nocheck - private function to add an attribute to
 * 				  netlink message without buffer len check
 *
 * @nlh:	pointer to netlink message
 * @type:	netlink attribute type that will be added
 * @len:	netlink attribute payload length
 * @data:	pointer to the data that will be stored by the attribute
 */
static inline void __sancus_nl_attr_put_nocheck(struct nlmsghdr *nlh,
						uint16_t type, uint16_t len,
						const void *data)
{
	struct nlattr *attr = sancus_nl_msg_get_payload_tail(nlh);
	unsigned payload_len = SANCUS_NL_ALIGN(sizeof(struct nlattr)) + len;

	attr->nla_type = type;
	attr->nla_len = (uint16_t)payload_len;
	memcpy(sancus_nl_attr_get_payload(attr), data, len);
	/* update nlmsg_len field */
	nlh->nlmsg_len += SANCUS_NL_ALIGN(payload_len);
}

/**
 * __sancus_nl_attr_put_check - private function to add an attribute to
 * 				netlink message with prior buffer len check
 *
 * @nlh:	pointer to netlink message
 * @buflen:	size of buffer for stores the message
 * @type:	netlink attribute type that will be added
 * @len:	netlink attribute payload length
 * @data:	pointer to the data that will be stored by the attribute
 */
static inline bool __sancus_nl_attr_put_check(struct nlmsghdr *nlh, size_t buflen,
			uint16_t type, uint16_t len, const void *data)
{
	/* check if attribute fits into the buffer */
	if (nlh->nlmsg_len + SANCUS_NL_ATTR_HDRLEN + SANCUS_NL_ALIGN(len) > buflen)
		return false;
	__sancus_nl_attr_put_nocheck(nlh, type, len, data);

	return true;
}

bool sancus_nl_attr_put(struct nlmsghdr *nlh, size_t buflen,
			uint16_t type, uint16_t len, const void *data)
{
	return __sancus_nl_attr_put_check(nlh, buflen, type, len, &data);
}

bool sancus_nl_attr_put_u8(struct nlmsghdr *nlh, size_t buflen, uint16_t type, uint8_t data)
{
	return __sancus_nl_attr_put_check(nlh, buflen, type, sizeof(uint8_t), &data);
}

bool sancus_nl_attr_put_u16(struct nlmsghdr *nlh, size_t buflen, uint16_t type, uint16_t data)
{
	return __sancus_nl_attr_put_check(nlh, buflen, type, sizeof(uint16_t), &data);
}

bool sancus_nl_attr_put_u32(struct nlmsghdr *nlh, size_t buflen, uint16_t type, uint32_t data)
{
	return __sancus_nl_attr_put_check(nlh, buflen, type, sizeof(uint32_t), &data);
}

bool sancus_nl_attr_put_u64(struct nlmsghdr *nlh, size_t buflen, uint16_t type, uint64_t data)
{
	return __sancus_nl_attr_put_check(nlh, buflen, type, sizeof(uint64_t), &data);
}

bool sancus_nl_attr_put_string(struct nlmsghdr *nlh, size_t buflen, uint16_t type, const char *data)
{
	return __sancus_nl_attr_put_check(nlh, buflen, type, sancus_u16zu(strlen(data)), data);
}

bool sancus_nl_attr_put_nul_string(struct nlmsghdr *nlh, size_t buflen, uint16_t type, const char *data)
{
	return __sancus_nl_attr_put_check(nlh, buflen, type, sancus_u16zu(strlen(data)+1), data);
}

