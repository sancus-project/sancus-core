AM_CFLAGS = -I @top_srcdir@/include @EV_CFLAGS@

lib_LTLIBRARIES = libsancus-core.la

libsancus_core_la_SOURCES = \
	alloc.c \
	buffer.c \
	fmt_cstr.c \
	log.c \
	sancus_serial.c \
	stream.c \
	tcp_conn.c \
	tcp_server.c

libsancus_core_la_LIBADD = @EV_LIBS@
