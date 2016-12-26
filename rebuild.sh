#!/bin/sh
set -e
set -x
export CFLAGS="-Wall -Werror -ggdb3"
cd `dirname $0`
mkdir -p build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/var/tmp/unshield .. && make && make install
