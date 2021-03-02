AC_INIT([sancus-core], [0.3.10.2], [amery@linux.com])

AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign subdir-objects dist-xz -Wall -Werror])

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

dnl Check Compiler features
dnl
AX_GCC_BUILTIN(__builtin_expect)
AX_GCC_VAR_ATTRIBUTE(unused)
AX_GCC_FUNC_ATTRIBUTE(const)
AX_GCC_FUNC_ATTRIBUTE(pure)
AX_GCC_FUNC_ATTRIBUTE(noreturn)
AX_GCC_FUNC_ATTRIBUTE(format)

dnl Options
dnl

dnl --enable-release
dnl
AC_MSG_CHECKING([whether to build in release mode])
AC_ARG_ENABLE([release],
	      [AS_HELP_STRING([--enable-release], [build in release mode])],
	      [is_release="$enableval"],
	      [is_release=no])
AC_MSG_RESULT([$is_release])

if test "x$is_release" = "xyes"; then
	dnl -DNDEBUG
	CPPFLAGS="$CPPFLAGS -DNDEBUG"
else
	AC_MSG_CHECKING([whether to build in debug mode])
	AC_ARG_ENABLE([debug],
		      [AS_HELP_STRING([--enable-debug], [build in debug mode])],
		      [is_debug="$enableval"],
		      [is_debug=no])
	AC_MSG_RESULT([$is_debug])

	dnl -g -O0
	dnl
	if test "x$is_debug" = "xyes"; then
		dnl Remove all optimization flags from CFLAGS
		changequote({,})
		CFLAGS=`echo "$CFLAGS" | $SED -e 's/-O[0-9s]*//g'`
		CXXFLAGS=`echo "$CXXFLAGS" | $SED -e 's/-O[0-9s]*//g'`

		CFLAGS=`echo "$CFLAGS" | $SED -e 's/-g[0-9]*//g'`
		CXXFLAGS=`echo "$CXXFLAGS" | $SED -e 's/-g[0-9]*//g'`
		changequote([,])

		CFLAGS="$CFLAGS -g -O0"
		CXXFLAGS="$CXXFLAGS -g -O0"
	fi

	AC_MSG_CHECKING([whether to allow assert to abort])
	AC_ARG_ENABLE([assert],
		      [AS_HELP_STRING([--disable-assert], [disable abort and trap calls])],
		      [can_assert="$enableval"],
		      [can_assert=yes])
	AC_MSG_RESULT([$can_assert])

	dnl -DNDEBUG
	if test "x$can_assert" = "xno"; then
		CPPFLAGS="$CPPFLAGS -DNDEBUG"
	fi
fi

AM_CONDITIONAL([IS_RELEASE], [test "x$is_release" = xyes])
AM_CONDITIONAL([IS_DEBUG], [test "x$is_debug" = xyes])

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
