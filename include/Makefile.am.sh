#!/bin/sh

list() {
	echo "$@" | tr ' ' '\n' | sort -V | tr '\n' '|' |
		sed -e 's,|$,,' -e 's,|, \\\n\t,g'
}

cd "${0%/*}"
cat <<EOT | tee Makefile.am
include_HEADERS = \\
	$(list *.h)

if HAVE_LINUX_NETLINK
include_HEADERS += netlink/sancus_netlink.h
endif
EOT
