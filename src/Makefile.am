AM_CFLAGS = @WARN_CFLAGS@
AM_CPPFLAGS = -I @top_srcdir@/include

if IS_DEBUG
AM_CFLAGS += -Wnull-dereference -Wformat -Werror
endif

lib_LTLIBRARIES = libsancus-core.la

libsancus_core_la_SOURCES = \
	sancus/alloc.c \
	sancus/buffer.c \
	sancus/buffer_legacy.c \
	sancus/clock.c \
	sancus/fd.c \
	sancus/fmt_cstr.c \
	sancus/logger.c \
	sancus/sancus_serial.c \
	sancus/stream.c \
	sancus/tcp_conn.c \
	sancus/tcp_server.c \
	sancus/time.c

libsancus_core_la_LDFLAGS = -Wl,--no-undefined

if HAVE_LINUX_NETLINK
lib_LTLIBRARIES += libsancus-netlink.la

libsancus_netlink_la_SOURCES = \
	netlink/netlink_attr.c \
	netlink/netlink_io.c \
	netlink/netlink_msg.c

libsancus_netlink_la_LDFLAGS = -Wl,--no-undefined
endif

EXTRA_DIST =

# tests
#
TESTS =
testdir = $(libexecdir)/sancus
test_PROGRAMS =

# test-time
#
TESTS += test-time
test_PROGRAMS += test-time
test_time_SOURCES = tests/time.c
test_time_CPPFLAGS = $(AM_CPPFLAGS) '-DTEST_NAME="time-test"'
test_time_LDADD = libsancus-core.la
