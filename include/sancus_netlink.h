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
#ifndef _SANCUS_NETLINK_H
#define _SANCUS_NETLINK_H

/**
 * Callback API
 */

#define SANCUS_NL_CB_ERROR      -1
#define SANCUS_NL_CB_STOP        0
#define SANCUS_NL_CB_OK          1

typedef int (*sancus_nl_attr_parse_cb)(const struct nlattr *attr, void *data);


struct sancus_nl_receiver;

/**
 * enum sancus_nl_receiver_error - list of possible errors
 *
 * @SANCUS_NL_RECEIVER_WATCHER_ERROR:	receive watcher error, server has been closed
 */
enum sancus_nl_receiver_error {
	SANCUS_NL_RECEIVER_WATCHER_ERROR,
	SANCUS_NL_RECEIVER_RECVFROM_ERROR,
};

/**
 * struct sancus_receiver_settings - driving callbacks of netlink receiver
 *
 * @on_message:		callback triggered on each new message
 * @on_attribute:	callback triggered on each new message
 * @on_error:		an error has happened, tell the world
 * @attribute_offset:	offset within the message where attributes start
 */
struct sancus_nl_receiver_settings {
	bool (*on_message) (struct sancus_nl_receiver *, struct ev_loop *, const struct nlmsghdr *);

	void (*on_error) (struct sancus_nl_receiver *,
			  struct ev_loop *,
			  enum sancus_nl_receiver_error);

	size_t attribute_offset;
};

/**
 * struct sancus_nl_receiver - netlink receiver
 *
 * @recv_watcher:	receive watcher
 * @settings:		driving callbacks
 * @portid:		port ID
 */
struct sancus_nl_receiver {
	struct ev_io recv_watcher;

	const struct sancus_nl_receiver_settings *settings;

	pid_t portid;
};

/**
 * sancus_nl_receiver_start - start watching netlink socket
 *
 * @self:	server to be started
 * @loop:	event loop
 *
 * The server shall not be already active. Nothing returned
 */
void sancus_nl_receiver_start(struct sancus_nl_receiver *self, struct ev_loop *loop);

/**
 * sancus_nl_receiver_stop - stop watching netlink socket
 * @self:	server to be stopped
 * @loop:	event loop
 *
 * The server shall not be already stopped. Nothing returned
 */
void sancus_nl_receiver_stop(struct sancus_nl_receiver *self, struct ev_loop *loop);

/**
 * sancus_nl_receiver_close - closes an already stopped netlink receiver
 *
 * @self:	server to close
 */
void sancus_nl_receiver_close(struct sancus_nl_receiver *self);

/**
 * sancus_nl_receiver_listen - initializes and prepares netlink receiver
 *
 * @self:	server structure to initialize
 * @settings:	driving callbacks
 * @bus:	netlink socket bus ID (see NETLINK_* constants)	
 * @groups:	the group of message you're interested in
 * @portid:	port ID you want to use (use zero for automatic selection)
 *
 * Returns 0 if @addr is invalid, 1 on success and -1 on error. errno set
 * accordingly.
 */
int sancus_nl_receiver_listen(struct sancus_nl_receiver *self,
			   const struct sancus_nl_receiver_settings *settings,
			   int bus, unsigned int groups, pid_t portid);
/**
 * SANCUS_NL_SOCKET_BUFFER_SIZE - netlink socket buffer size
 */
#define SANCUS_NL_SOCKET_BUFFER_SIZE (getpagesize() < 8192L ? getpagesize() : 8192L)

/**
 * sancus_nl_receiver_fd - returns fd been listened by a netlink receiver
 *
 * @S:	server structure to query
 */
#define sancus_nl_receiver_fd(S)		(S)->recv_watcher.fd

/**
 * sancus_nl_receiver_is_active - checks if a server has not been closed nor stoped
 *
 * @S:	server structure to query
 */
#define sancus_nl_receiver_is_active(S)	((S)->recv_watcher.fd > 0 && ev_is_active(&(S)->recv_watcher))



/**
 * Netlink Message handling
 */

/* Netlink message headers and its attributes are always aligned to four bytes. */
#define SANCUS_NL_ALIGNTO		4
#define SANCUS_NL_ALIGN(len)	(((len)+SANCUS_NL_ALIGNTO-1) & ~(SANCUS_NL_ALIGNTO-1))
#define SANCUS_NL_MSG_HDRLEN	SANCUS_NL_ALIGN(sizeof(struct nlmsghdr))

/**
 * sancus_nl_msg_ok - check if there is room for a netlink message
 * @nlh:	netlink message that we want to check
 * @len:	remaining bytes in a buffer that contains the netlink message
 */
bool sancus_nl_msg_ok(const struct nlmsghdr *nlh, int len);

/**
 * sancus_nl_msg_next - get the next netlink message out of a multipart message
 * @nlh:	netlink message that is currently handled
 * @len:	length of the remaining bytes in the buffer
 */
struct nlmsghdr *sancus_nl_msg_next(const struct nlmsghdr *nlh, int *len);

/**
 * sancus_nl_msg_portid_ok - perform a check if sending port ID is correct
 * @nlh:	netlink message that is currently handled
 * @portid:	netlink portid to check
 */
bool sancus_nl_msg_portid_ok(const struct nlmsghdr *nlh, unsigned int portid);

/**
 * sancus_nl_msg_get_payload - get a pointer to the payload of the netlink message
 * @nlh:	netlink message to get the payload from
 */
void *sancus_nl_msg_get_payload(const struct nlmsghdr *nlh);

/**
 * sancus_nl_msg_get_payload_offset - get a pointer to the payload at a given offset of the netlink message
 * @nlh:	netlink message to get the payload from
 */
void *sancus_nl_msg_get_payload_offset(const struct nlmsghdr *nlh, size_t offset);

/**
 * sancus_nl_msg_get_payload_tail - get a pointer to the end of the netlink message
 * @nlh:	pointer to a netlink message
 */
void *sancus_nl_msg_get_payload_tail(const struct nlmsghdr *nlh);


/**
 * Netlink Attribute handling
 */

/* attribute data types */
enum sancus_nl_attr_data_type {
	SANCUS_NL_ATTR_TYPE_UNSPEC,
	SANCUS_NL_ATTR_TYPE_U8,
	SANCUS_NL_ATTR_TYPE_U16,
	SANCUS_NL_ATTR_TYPE_U32,
	SANCUS_NL_ATTR_TYPE_U64,
	SANCUS_NL_ATTR_TYPE_STRING,
	SANCUS_NL_ATTR_TYPE_FLAG,
	SANCUS_NL_ATTR_TYPE_MSECS,
	SANCUS_NL_ATTR_TYPE_NESTED,
	SANCUS_NL_ATTR_TYPE_NESTED_COMPAT,
	SANCUS_NL_ATTR_TYPE_NUL_STRING,
	SANCUS_NL_ATTR_TYPE_BINARY,
	SANCUS_NL_ATTR_TYPE_MAX,
};

#define SANCUS_NL_ATTR_HDRLEN	SANCUS_NL_ALIGN(sizeof(struct nlattr))

/**
 * sancus_nl_attr_get_type - get the type of a netlink attribute
 * @attr:	pointer to a netlink attribute
 */
uint16_t sancus_nl_attr_get_type(const struct nlattr *attr);

/**
 * sancus_nl_attr_get_len - get the length of a netlink attribute
 * @attr:	pointer to a netlink attribute
 */
uint16_t sancus_nl_attr_get_len(const struct nlattr *attr);

/**
 * sancus_nl_attr_get_payload_len - get the payload length of a netlink attribute
 * @attr:	pointer to a netlink attribute
 */
uint16_t sancus_nl_attr_get_payload_len(const struct nlattr *attr);

/**
 * sancus_nl_attr_get_payload - get the payload of a netlink attribute
 * @attr:	pointer to a netlink attribute
 */
void *sancus_nl_attr_get_payload(const struct nlattr *attr);

/**
 * sancus_nl_attr_ok - check if there is space for an attribute in a buffer
 * @attr:	attribute to be checked
 * @len:	remaining bytes in the buffer that contains the attribute
 */
bool sancus_nl_attr_ok(const struct nlattr *attr, int len);

/**
 * sancus_nl_attr_next - get the next attribute in the payload of a netlink message
 * @attr:	pointer to the current attribute
 */
struct nlattr *sancus_nl_attr_next(const struct nlattr *attr);

/**
 * sancus_nl_attr_type_valid - check if the attribute type is valid
 * @attr:	pointer to attribute
 * @max:	maximum attribute type
 */
int sancus_nl_attr_type_valid(const struct nlattr *attr, uint16_t max);

/**
 * sancus_nl_attr_validate_minlen - validate netlink attribute based on the
 * 				    minimal length of the data type
 * @attr:	netlink attribute
 * @type:	attribute type
 *
 * The basic validation is based on the minimal length a data type will occupy.
 * Specifically, it checks that integer the types (u8, u16, u32 and u64) will
 * have enough space and that string types do not have an empty payload.
 * This function returns -1 in case of an error setting errno appropriately.
 */
int sancus_nl_attr_validate_minlen(const struct nlattr *attr,
				   enum sancus_nl_attr_data_type type);

/**
 * sancus_nl_attr_foreach - iterate over the attributes of a netlink message
 * @attr:	pointer to the current attribute
 * @nlh:	pointer to the netlink message
 * @offset:	offset within the message where attributes start
 */
#define sancus_nl_attr_foreach(attr, nlh, offset) \
	for ((attr) = sancus_nl_msg_get_payload_offset((nlh), (offset)); \
	     sancus_nl_attr_ok((attr), (char *)sancus_nl_msg_get_payload_tail(nlh) - (char *)(attr)); \
	     (attr) = sancus_nl_attr_next(attr))


int sancus_nl_attr_parse(const struct nlmsghdr *nlh, unsigned int offset,
			 sancus_nl_attr_parse_cb cb, void *data);

/**
 * sancus_nl_attr_get_u8 - return a 8-bit unsigned integer attribute
 * @attr:	pointer to attribute
 */
uint8_t sancus_nl_attr_get_u8(const struct nlattr *attr);

/**
 * sancus_nl_attr_get_u16 - return a 16-bit unsigned integer attribute
 * @attr:	pointer to attribute
 */
uint16_t sancus_nl_attr_get_u16(const struct nlattr *attr);

/**
 * sancus_nl_attr_get_u32 - return a 32-bit unsigned integer attribute
 * @attr:	pointer to attribute
 */
uint32_t sancus_nl_attr_get_u32(const struct nlattr *attr);

/**
 * sancus_nl_attr_get_u64 - return a 64-bit unsigned integer attribute
 * @attr:	pointer to attribute
 */
uint64_t sancus_nl_attr_get_u64(const struct nlattr *attr);

/**
 * sancus_nl_attr_get_str - return a string attribute
 * @attr:	pointer to attribute
 */
const char *sancus_nl_attr_get_str(const struct nlattr *attr);

#endif /* !_SANCUS_NETLINK_H */
