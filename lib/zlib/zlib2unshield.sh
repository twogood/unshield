#!/bin/sh

# must not use ./configure --solo
# Z_SOLO makes functions behave differently

# linux
git checkout -- .
./configure

sed -i '/OBJG = /d' Makefile

#deflate='deflate.c deflate.h trees.c trees.h'

mkdir -p zlib-nogz
cp -fv ${deflate} \
	adler32.c \
	crc32.c crc32.h \
	infback.c \
	inffast.c inffast.h \
	inffixed.h \
	inflate.c inflate.h \
	inftrees.c inftrees.h \
	zutil.c zutil.h \
	zconf.h zlib.h \
	zlib-nogz

# copy zlib-nogz/* to unshield/lib/zlib/
# try to compile: run ./rebuild.sh and fix any issues until it compiles