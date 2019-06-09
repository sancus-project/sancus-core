#!/bin/sh

list() {
	sort -V | tr '\n' '|' |
		sed -e 's,|$,,' -e 's,|, \\\n\t,g'
}

cd "${0%/*}"
cat <<EOT | tee Makefile.am
nobase_include_HEADERS = \\
	$(find * -name '*.h' | grep -v netlink | list)

if HAVE_LINUX_NETLINK
nobase_include_HEADERS += \\
	$(find * -name '*.h' | grep netlink | list)
endif
EOT
