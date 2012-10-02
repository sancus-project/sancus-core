#!/bin/sh

list() {
	echo "$@" | tr ' ' '\n' | sort -V | tr '\n' '|' |
		sed -e 's,|$,,' -e 's,|, \\\n\t,g'
}

cd "${0%/*}"
cat <<EOT | tee Makefile.am
AM_CFLAGS = -I @top_srcdir@/include @EV_CFLAGS@

lib_LTLIBRARIES = libsancus.la

libsancus_la_SOURCES = \\
	$(list *.c)

libsancus_la_LIBADD = @EV_LIBS@
EOT
