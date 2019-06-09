#!/bin/sh

set -eu

list() {
	sort -V | tr '\n' '|' |
		sed -e 's,|$,,' -e 's,|, \\\n\t,g'
}

F="${0##*/}"
F="${F%.sh}"

cd "${0%/*}"
cat <<EOT > $F~
nobase_include_HEADERS = \\
	$(find * -name '*.h' | grep -v netlink | list)

if HAVE_LINUX_NETLINK
nobase_include_HEADERS += \\
	$(find * -name '*.h' | grep netlink | list)
endif
EOT
mv $F~ $F
