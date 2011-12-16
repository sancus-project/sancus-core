#!/bin/sh

list() {
	echo "$@" | sort -V | fmt -w60 | tr '\n' '|' |
		sed -e 's,|$,,' -e 's,|, \\\n\t,g'
}

cd "${0%/*}"
cat <<EOT | tee Makefile.am
AM_CFLAGS = -I @top_srcdir@/include

lib_LTLIBRARIES = libsancus.la

libsancus_la_SOURCES = \\
	$(list *.c)
EOT
