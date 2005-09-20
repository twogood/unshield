#!/bin/sh
set -x
export CFLAGS="-Wall -Werror -ggdb3"
./bootstrap &&
./configure --prefix=/var/tmp/unshield --with-ssl &&
make install
