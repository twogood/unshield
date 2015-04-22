#!/bin/sh
set -x
export CFLAGS="-Wall -Werror -ggdb3"
cmake -DCMAKE_INSTALL_PREFIX:PATH=/var/tmp/unshield . && make && make install
