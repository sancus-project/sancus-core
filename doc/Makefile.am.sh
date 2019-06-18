#!/bin/sh

F="${0##*/}"
F="${F%.sh}"

cd "${0%/*}"
cat <<EOT > $F~

TSTAMP = doxyfile.stamp

if HAVE_DOXYGEN

\$(TSTAMP): Doxyfile
	\$(DOXYGEN) $^
	touch \$@

CLEANFILES = \$(TSTAMP)
all-local: \$(TSTAMP)

endif
EOT
mv $F~ $F
