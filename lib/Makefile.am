## useful flags
AM_CFLAGS = -g -ansi -Wall -Wno-long-long -Werror @CFLAGS@ @APPLE_CFLAGS@ -I..

## create this library
lib_LTLIBRARIES = libunshield.la

## use these sources
libunshield_la_SOURCES = \
	directory.c \
	file.c \
	helper.c \
	unshield.h unshield_internal.h \
	unshield.c

libunshield_la_LDFLAGS = -no-undefined -version-info 0:0:0

include_HEADERS = unshield.h

