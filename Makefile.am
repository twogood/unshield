## Makefile.am -- Process this file with automake to produce Makefile.in

SUBDIRS = lib src

ACLOCAL_AMFLAGS = -I autoconf-m4

pkgconfigdir   = $(libdir)/pkgconfig
pkgconfig_DATA = libunshield.pc

man_MANS = man/unshield.1

EXTRA_DIST = libunshield.pc.in LICENSE $(man_MANS)
CLEANFILES = libunshield.pc
