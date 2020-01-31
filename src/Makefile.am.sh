#!/bin/sh

set -eu

F="${0##*/}"
F="${F%.sh}"
BASE="$(dirname "$0")"

echo "Generating $BASE/$F" >&2

list() {
	sort -uV | tr '\n' '|' |
		sed -e 's,|$,,' -e 's,|, \\\n\t,g' -e 's|^|\t|'
}

list_files() {
	local k="$1"; shift
	local x=

	if [ $# -gt 1 ]; then
		cat <<EOT
$k = \\
EOT
		for x; do
			echo "$x"
		done | list
	elif [ $# -eq 1 ]; then
		cat <<EOT
$k = $1
EOT
	else
		cat <<EOT
$k =
EOT
	fi
}

list_find_files() {
	local k="$1"; shift
	list_files "$k" $(find "$@" 2> /dev/null)
}

cd "$BASE"
cat <<EOT > $F~
AM_CFLAGS = @WARN_CFLAGS@
AM_CPPFLAGS = -I @top_srcdir@/include

if IS_DEBUG
AM_CFLAGS += -Wnull-dereference -Wformat -Werror
endif

lib_LTLIBRARIES = libsancus-core.la

$(list_find_files libsancus_core_la_SOURCES sancus/ -name '*.c')

libsancus_core_la_LDFLAGS = -Wl,--no-undefined

if HAVE_LINUX_NETLINK
lib_LTLIBRARIES += libsancus-netlink.la

$(list_find_files libsancus_netlink_la_SOURCES netlink/ -name '*.c')

libsancus_netlink_la_LDFLAGS = -Wl,--no-undefined
endif

$(list_find_files EXTRA_DIST * -name '*.h')

# tests
#
TESTS =
testdir = \$(libexecdir)/sancus
test_PROGRAMS =
EOT

find tests -name '*.c' 2> /dev/null 2> /dev/null | cut -d/ -f2 | sort -uV | while read f; do

	k="${f%.c}"

	n="test-$k"
	N="$(echo $n | tr '-' '_')"
	cat <<EOT >> $F~

# $n
#
TESTS += $n
test_PROGRAMS += $n
$(list_find_files ${N}_SOURCES tests/$f -name '*.c')
${N}_CPPFLAGS = \$(AM_CPPFLAGS) '-DTEST_NAME="$k-test"'
${N}_LDADD = libsancus-core.la
EOT
done

mv $F~ $F
