AM_CFLAGS = -I @top_srcdir@/include

lib_LTLIBRARIES = libsancus-core.la

libsancus_core_la_SOURCES = \
	alloc.c \
	buffer.c \
	fmt_cstr.c \
	log.c \
	sancus_serial.c \
	stream.c \
	tcp_conn.c \
	tcp_server.c \
	time.c

libsancus_core_la_LDFLAGS = -Wl,--no-undefined

if HAVE_LINUX_NETLINK
lib_LTLIBRARIES += libsancus-netlink.la

libsancus_netlink_la_SOURCES = \
	netlink/netlink_attr.c \
	netlink/netlink_io.c \
	netlink/netlink_msg.c

libsancus_netlink_la_LDFLAGS = -Wl,--no-undefined
endif

TESTS =
testdir = $(libexecdir)/sancus
test_PROGRAMS =

# test-time
#
TESTS += test-time
test_PROGRAMS += test-time
test_time_SOURCES = tests/time.c
