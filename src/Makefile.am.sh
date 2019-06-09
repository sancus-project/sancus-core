#!/bin/sh

set -eu
list() {
	echo "$@" | tr ' ' '\n' | sort -V | tr '\n' '|' |
		sed -e 's,|$,,' -e 's,|, \\\n\t,g'
}

F="${0##*/}"
F="${F%.sh}"

cd "${0%/*}"
cat <<EOT > $F~
AM_CFLAGS = -I @top_srcdir@/include

lib_LTLIBRARIES = libsancus-core.la

libsancus_core_la_SOURCES = \\
	$(list *.c)

libsancus_core_la_LDFLAGS = -Wl,--no-undefined
EOT
mv $F~ $F
