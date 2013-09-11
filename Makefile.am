ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src include doc

if HAVE_LINUX_NETLINK
SUBDIRS += src/netlink
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = @PACKAGE_NAME@.pc

EXTRA_DIST = LICENSE
