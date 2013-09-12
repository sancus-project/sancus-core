#!/bin/sh

list() {
	echo "$@" | tr ' ' '\n' | sort -V | tr '\n' '|' |
		sed -e 's,|$,,' -e 's,|, \\\n\t,g'
}

cd "${0%/*}"
cat <<EOT | tee Makefile.am
AM_CFLAGS = -I @top_srcdir@/include

if HAVE_LINUX_NETLINK
AM_CFLAGS += -I @top_srcdir@/include/netlink
endif

lib_LTLIBRARIES = libsancus-netlink.la

libsancus_netlink_la_SOURCES = \\
	$(list *.c)
EOT
