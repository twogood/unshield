#!/bin/sh

set_md5sum()
{
    if test -n "$MD5SUM" ; then
        return # already set
    fi
    if command -v md5sum >/dev/null 2>&1 ; then
        export MD5SUM=md5sum
    elif command -v gmd5sum >/dev/null 2>&1 ; then
        export MD5SUM=gmd5sum
    else
        echo "md5sum utility is missing, please install it"
        echo "aborting tests"
        exit 1
    fi
}


set_unshield()
{
    if test -n "$UNSHIELD" ; then
        return # already set
    fi
    unshields="$(pwd)/unshield $(pwd)/unshield.exe
$(pwd)/src/unshield
$(pwd)/build/src/unshield
$(pwd)/src/unshield.exe
$(pwd)/build/src/unshield.exe"
    for i in ${unshields}
    do
        if ! test -x "$i" ; then
            continue
        fi
        echo "------------------------------------------------"
        echo "UNSHIELD=${i}"
        echo "------------------------------------------------"
        if test "$i" = "$(pwd)/unshield" || test "$i" = "$(pwd)/unshield" ; then
           echo "*** using custom unshield binary in ./"
           echo "------------------------------------------------"
        fi
        export UNSHIELD=${i}
        break
    done
    sleep 1
    if test -z "$UNSHIELD" ; then
        echo "coult not find unshield in the following locations:"
        echo "${unshields}"
        echo "aborting"
        exit 1
    fi
}


set_directory()
{
    DIR="$1"
    mkdir -p "$DIR"
    cd "$DIR" || exit 1
}


download_file()
{
    dlurl="$1"
    dlfile="$2"
    if ! test -f "${dlfile}" ; then
        if command -v curl >/dev/null 2>&1 ; then
            curl -sSL -o "${dlfile}" "${dlurl}"
        else
            wget -q --no-check-certificate -O "${dlfile}" "${dlurl}"
        fi
    fi
}


clean_directory_except()
{
    keepfile="$1"
    cd "$DIR"
    for i in $(ls)
    do
        if test "$i" != "$keepfile" ; then
            rm -rf "$i"
        fi
    done
}


cleanup_func()
{
    if test -z "$KEEP_TESTS" ; then
        trap "$@" TERM INT EXIT
    fi
}

winFixMd5()
{
    if test "$TRAVIS_OS_NAME" = "windows" ; then # see rebuild.sh
        # problem with md5sum in Travis CI (mingw)
        #  unix: a943ad8f40479fa5cd68afba5787be4f  ./AVIGO/Avigo100.pgm
        #  win : a943ad8f40479fa5cd68afba5787be4f *./AVIGO/Avigo100.pgm
        sed -i 's% \*\./%  ./%' "${1}"
    fi
}
