AC_INIT(sancus-core, 0.1.10, amery@geeks.cl)

AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign subdir-objects dist-bzip2 -Wall -Werror])

AC_CONFIG_MACRO_DIR([m4])

# automake >= 1.12 requires AM_PROG_AR,
# but automake 1.11 doesn't recognize it
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

LT_INIT

dnl
dnl Check for programs
dnl
AC_PROG_CC_C99

test -z "$SED" && SED=sed

dnl
dnl Check for headers
dnl
AC_CHECK_HEADERS([linux/netlink.h], [have_linux_netlink=yes], [have_linux_netlink=no], [])
AM_CONDITIONAL([HAVE_LINUX_NETLINK], [test $have_linux_netlink = yes])
if test x"$have_linux_netlink" = x"yes"; then
	AC_CONFIG_FILES([sancus-netlink.pc])
fi

dnl
dnl Options
dnl

dnl --enable-debug
dnl
AC_MSG_CHECKING([wheter to build with debug information])
AC_ARG_ENABLE([debug],
	[AS_HELP_STRING([--enable-debug], [enable debug data generation (def=almost)])],
	[debugit="$enableval"],
	[debugit=almost])
AC_MSG_RESULT([$debugit])

dnl CFLAGS
dnl
if test x"$debugit" = x"yes"; then
	dnl Remove all optimization flags from CFLAGS
	changequote({,})
	CFLAGS=`echo "$CFLAGS" | $SED -e 's/-O[0-9s]*//g'`
	CXXFLAGS=`echo "$CXXFLAGS" | $SED -e 's/-O[0-9s]*//g'`

	CFLAGS=`echo "$CFLAGS" | $SED -e 's/-g[0-9]*//g'`
	CXXFLAGS=`echo "$CXXFLAGS" | $SED -e 's/-g[0-9]*//g'`
	changequote([,])

	CFLAGS="$CFLAGS -g -O0"
fi

dnl WARN_CFLAGS
AX_COMPILER_FLAGS_CFLAGS

dnl Doxygen
dnl
AC_CHECK_PROGS([DOXYGEN], [doxygen])
if test -z "$DOXYGEN"; then
	AC_MSG_WARN([Doxygen not found - documentation won't be generated])
fi
AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])

dnl Output
dnl
AC_CONFIG_HEADERS([src/config.h])

AC_CONFIG_FILES([sancus-core.pc])
AC_CONFIG_FILES([Makefile
		 doc/Makefile
		 doc/Doxyfile
		 include/Makefile
		 src/Makefile])

AC_OUTPUT
