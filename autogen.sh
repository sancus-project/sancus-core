#!/bin/sh

for x in ${0%/*}/*/Makefile.am.sh; do
	[ -s "$x" ] || continue
	$SHELL "$x"
done

mkdir -p "${0%/*}/m4"
exec autoreconf -ivs "${0%/*}"
