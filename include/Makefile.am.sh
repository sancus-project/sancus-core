#!/bin/sh

set -eu

F="${0##*/}"
F="${F%.sh}"
BASE="$(dirname "$0")"

echo "Generating $BASE/$F" >&2

list() {
	sort -V | tr '\n' '|' |
		sed -e 's,|$,,' -e 's,|, \\\n\t,g'
}

cd "$BASE"

cat <<EOT > $F~
nobase_include_HEADERS = \\
	$(find * -name '*.h' | grep -v netlink | list)

if HAVE_LINUX_NETLINK
nobase_include_HEADERS += \\
	$(find * -name '*.h' | grep netlink | list)
endif
EOT
mv $F~ $F
