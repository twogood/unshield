## useful flags
AM_CFLAGS = -g -ansi -Wall -Wsign-compare -Wno-long-long -Werror @CFLAGS@ @APPLE_CFLAGS@ -I..

## create this library
lib_LTLIBRARIES = libunshield.la

## use these sources
libunshield_la_SOURCES = \
	bswap.c \
	component.c \
	directory.c \
	file.c \
	helper.c \
	internal.h \
	libunshield.h libunshield.c \
	log.h log.c

libunshield_la_LDFLAGS = -no-undefined -version-info 0:0:0
libunshield_la_LIBADD  = md5/libmd5.la

include_HEADERS = libunshield.h

SUBDIRS = md5 .
