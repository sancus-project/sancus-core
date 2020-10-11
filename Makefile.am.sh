#!/bin/sh

set -eu

BASE="$(dirname "$0")"
for x in "$BASE"/*/Makefile.am.sh; do
	[ ! -x "$x" ] || "$x"
done
