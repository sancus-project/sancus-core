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
	$(find * -name '*.c' | grep -v -e '^\(netlink\|tests\)/' | list)

libsancus_core_la_LDFLAGS = -Wl,--no-undefined

if HAVE_LINUX_NETLINK
lib_LTLIBRARIES += libsancus-netlink.la

libsancus_netlink_la_SOURCES = \\
	$(find netlink/ -name '*.c' | list)

libsancus_netlink_la_LDFLAGS = -Wl,--no-undefined
endif

# private headers
#
EXTRA_DIST = \\
	$(find -name '*.h' | list)

# tests
#
TESTS =
testdir = \$(libexecdir)/sancus
test_PROGRAMS =
EOT

find tests -name '*.c' 2> /dev/null 2> /dev/null | cut -d/ -f2 | sort -uV | while read f; do
	n="test-${f%.c}"
	N="$(echo $n | tr '-' '_')"
	cat <<EOT >> $F~

# $n
#
TESTS += $n
test_PROGRAMS += $n
${N}_SOURCES = $(find tests/$f -name '*.c' | list)
${N}_LDADD = libsancus-core.la
EOT
done

mv $F~ $F
