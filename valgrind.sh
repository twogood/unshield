#!/bin/sh
libtool --mode=execute valgrind --num-callers=10 --leak-check=yes `dirname $0`/src/unshield $@
