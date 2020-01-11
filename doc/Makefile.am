
TSTAMP = doxyfile.stamp

if HAVE_DOXYGEN

$(TSTAMP): Doxyfile
	$(DOXYGEN) $^
	touch $@

CLEANFILES = $(TSTAMP)
all-local: $(TSTAMP)

clean-local:
	rm -rf html man latex

endif
