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
    export UNSHIELD=/var/tmp/unshield/bin/unshield
    if ! test -x ${UNSHIELD} ; then
        echo "unshield executable not found at $UNSHIELD" >&2
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
