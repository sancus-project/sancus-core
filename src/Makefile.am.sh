#!/bin/sh

set -eu
list() {
	sort -uV | tr '\n' '|' |
		sed -e 's,|$,,' -e 's,|, \\\n\t,g'
}

F="${0##*/}"
F="${F%.sh}"

cd "${0%/*}"
cat <<EOT > $F~
AM_CFLAGS = -I @top_srcdir@/include

lib_LTLIBRARIES = libsancus-core.la

libsancus_core_la_SOURCES = \\
	$(find * -name '*.c' | grep -v -e '^netlink/' | list)

libsancus_core_la_LDFLAGS = -Wl,--no-undefined

if HAVE_LINUX_NETLINK
lib_LTLIBRARIES += libsancus-netlink.la

libsancus_netlink_la_SOURCES = \\
	$(find netlink/ -name '*.c' | list)

libsancus_netlink_la_LDFLAGS = -Wl,--no-undefined
endif
EOT
mv $F~ $F
