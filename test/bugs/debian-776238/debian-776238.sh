#!/bin/bash
set -e
cd `dirname $0`
MD5_FILE=`pwd`/`basename $0 .sh`.md5
CAB_FILE=`pwd`/data1.cab
UNSHIELD=${UNSHIELD:-/var/tmp/unshield/bin/unshield}

if [ \! -x ${UNSHIELD} ]; then
    echo "unshield executable not found at $UNSHIELD" >&2
    exit 1
fi

DIR=`mktemp -d`
#trap 'rm -rf ${DIR}' TERM INT EXIT
cd ${DIR}

set +e
rm -f /tmp/moo 

timeout 10 ${UNSHIELD} -d extract1 x "$CAB_FILE" > log1 2>&1
CODE=$?
if [ -e /tmp/moo ]; then
    cat log1 >&2
    echo "unshield vulnerable to CVE-2015-1386" >&2
    echo "See https://github.com/twogood/unshield/issues/42" >&2
    exit 2
fi

if [ ${CODE} -ne 1 ]; then
    cat log1 >&2
    echo "unshield should have failed with error 1 but was $CODE" >&2
    exit 3
fi

exit 0
