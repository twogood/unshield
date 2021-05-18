#!/bin/sh
# no param = cmake, travis CI = cmake

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
        .github/travis_windows_msvc.bat
    else
        export CFLAGS="-Wall" # gcc
        .github/travis_windows_mingw.bat
    fi
    exit $?
fi

# =====================================================================

rm -rf /var/tmp/unshield

if test -z "$1" ; then
    export CFLAGS="-Wall -Werror -ggdb3"
    mkdir -p build
    cd build
    cmake -DBUILD_STATIC=1 -DCMAKE_INSTALL_PREFIX:PATH=/var/tmp/unshield .. && \
        make && make install
else
    # param = autotools, proper cflags are in configure.ac
    if ! test -f configure ; then
        sh ./autogen.sh
    fi
    ./configure --prefix=/var/tmp/unshield --enable-static --disable-shared && \
    make clean && \
    make install
fi

