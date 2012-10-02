#!/bin/sh

list() {
	echo "$@" | tr ' ' '\n' | sort -V | tr '\n' '|' |
		sed -e 's,|$,,' -e 's,|, \\\n\t,g'
}

cd "${0%/*}"
cat <<EOT | tee Makefile.am
include_HEADERS = \\
	$(list *.h)
EOT
