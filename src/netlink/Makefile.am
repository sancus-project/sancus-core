AM_CFLAGS = -I @top_srcdir@/include

if HAVE_LINUX_NETLINK
AM_CFLAGS += -I @top_srcdir@/include/netlink
endif

lib_LTLIBRARIES = libsancus-netlink.la

libsancus_netlink_la_SOURCES = \
	netlink_attr.c \
	netlink_io.c \
	netlink_msg.c
