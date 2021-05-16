#!/bin/sh
set -e
set -x

if test "$TRAVIS_OS_NAME" = "windows" ; then
    # see .travis.yml
    if test "$TRAVIS_COMPILER" = "clang" ; then
        # https://clang.llvm.org/docs/MSVCCompatibility.html
        # clang on Windows uses MSVC stuff but it's not recognize but CMakeLists
        # so must use MSVC instead, just unset some vars
        echo "*** Will use MSVC ***"
        export CFLAGS="-W3"
        unset CC CC_FOR_BUILD CXX CXX_FOR_BUILD
    else
        export CFLAGS="-Wall" # gcc
    fi
    .github/travis_windows_build.bat
    exit $?
fi

export CFLAGS="-Wall -Werror -ggdb3"
cd `dirname $0`
mkdir -p build
cd build
cmake -DBUILD_STATIC=ON -DCMAKE_INSTALL_PREFIX:PATH=/var/tmp/unshield .. && make && make install
