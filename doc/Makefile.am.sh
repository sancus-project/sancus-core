#!/bin/sh

cd "${0%/*}"
cat <<EOT | tee Makefile.am

TSTAMP = doxyfile.stamp

if HAVE_DOXYGEN

\$(TSTAMP): Doxyfile
	\$(DOXYGEN) $^
	touch \$@

CLEANFILES = \$(TSTAMP)
all-local: \$(TSTAMP)

endif
EOT
