AC_INIT(sancus-core, 0.1.4, amery@geeks.cl)

AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign subdir-objects dist-bzip2 -Wall -Werror])

AC_CONFIG_MACRO_DIR([m4])

# automake >= 1.12 requires AM_PROG_AR,
# but automake 1.11 doesn't recognize it
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

LT_INIT

#
# Check for programs
#

AC_PROG_CC_C99

#
# Check for headers
#

# AC_CONFIG_HEADERS([src/config.h])
AC_CHECK_HEADERS([linux/netlink.h], [have_linux_netlink=yes], [have_linux_netlink=no], [])
AM_CONDITIONAL([HAVE_LINUX_NETLINK], [test $have_linux_netlink = yes])
if test x"$have_linux_netlink" = x"yes"; then
	AC_CONFIG_FILES([sancus-netlink.pc])
fi

#
# Options
#

# --enable-debug
#
AC_MSG_CHECKING([wheter to build with debug information])
AC_ARG_ENABLE([debug],
	[AS_HELP_STRING([--enable-debug], [enable debug data generation (def=almost)])],
	[debugit="$enableval"],
	[debugit=almost])
AC_MSG_RESULT([$debugit])

# CFLAGS
if test x"$debugit" = x"yes"; then
	AC_DEFINE([DEBUG],[],[Debug Mode])
	CFLAGS="-g -O0"
elif test x"$debugit" = x"no"; then
	AC_DEFINE([NDEBUG],[],[No-debug Mode])
	CFLAGS="-O3"
else
	CFLAGS="-O2"
fi

if test "$ac_cv_prog_cc_c99" != "no"; then
	CFLAGS="$CFLAGS $ac_cv_prog_cc_c99"
fi

CFLAGS="$CFLAGS -Wall -Wextra -pedantic -Winline"

dnl Doxygen
dnl
AC_CHECK_PROGS([DOXYGEN], [doxygen])
if test -z "$DOXYGEN"; then
	AC_MSG_WARN([Doxygen not found - documentation won't be generated])
fi
AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])

AC_CONFIG_FILES([sancus-core.pc])
AC_CONFIG_FILES([Makefile
		 doc/Makefile
		 doc/Doxyfile
		 include/Makefile
		 src/Makefile])
AC_OUTPUT
