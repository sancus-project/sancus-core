#!/bin/sh

list() {
	echo "$@" | tr ' ' '\n' | sort -V | tr '\n' '|' |
		sed -e 's,|$,,' -e 's,|, \\\n\t,g'
}

cd "${0%/*}"
cat <<EOT | tee Makefile.am
AM_CFLAGS = -I @top_srcdir@/include

lib_LTLIBRARIES = libsancus_netlink.la

libsancus_netlink_la_SOURCES = \\
	$(list *.c)
EOT
