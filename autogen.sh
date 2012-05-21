#!/bin/sh

case "$0" in
*/*)	BASE="${0%/*}" ;;
*)	BASE=. ;;
esac

for x in ${BASE}/*/Makefile.am.sh; do
	[ -s "$x" ] || continue
	$SHELL "$x"
done

mkdir -p "${BASE}/m4"
exec autoreconf -ivs "${BASE}"
