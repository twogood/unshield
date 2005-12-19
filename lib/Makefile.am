if USE_OUR_OWN_MD5
MD5_SUBDIR = md5
MD5_LIBS = md5/libmd5.la
else
MD5_SUBDIR =
MD5_LIBS =
endif

## useful flags
AM_CFLAGS = -ansi -Wsign-compare -Wno-long-long @APPLE_CFLAGS@ -I..

## create this library
lib_LTLIBRARIES = libunshield.la

## use these sources
libunshield_la_SOURCES = \
	bswap.c \
	component.c \
	directory.c \
	file.c \
	file_group.c \
	helper.c \
	internal.h \
	libunshield.h libunshield.c \
	log.h log.c

libunshield_la_LDFLAGS = -no-undefined -version-info 0:0:0
libunshield_la_LIBADD  = $(MD5_LIBS)

include_HEADERS = libunshield.h

SUBDIRS = $(MD5_SUBDIR) .
