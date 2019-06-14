ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src include doc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = @PACKAGE_NAME@.pc

if HAVE_LINUX_NETLINK
pkgconfig_DATA += sancus-netlink.pc
endif

EXTRA_DIST = LICENSE
