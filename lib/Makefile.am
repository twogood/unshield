## useful flags
AM_CFLAGS = -g -Wall -Wno-long-long -ansi @CFLAGS@ @APPLE_CFLAGS@ -I..

## create this library
lib_LTLIBRARIES = libunshield.la

## use these sources
libunshield_la_SOURCES = \
	unshield.h unshield_internal.h \
	unshield.c

libunshield_la_LDFLAGS = -no-undefined -version-info 0:0:0
libunshield_la_LIBADD  = @LTLIBICONV@ @LTLIBOBJS@

include_HEADERS = unshield.h

