#!/bin/bash
set -e
cd `dirname $0`
MD5_FILE=`pwd`/`basename $0 .sh`.md5
UNSHIELD=${UNSHIELD:-/var/tmp/unshield/bin/unshield}

if [ \! -x ${UNSHIELD} ]; then
    echo "unshield executable not found at $UNSHIELD" >&2
    exit 1
fi

DIR=`mktemp -d`
trap 'rm -rf ${DIR}' TERM INT EXIT
cd ${DIR}

#URL=https://www.ti.com/organizers/avigo/docs/avigomanager11b22.zip
URL="https://www.dropbox.com/s/8r4b6752swe3nhu/unshield-avigomanager11b22.zip?dl=1"
curl -sSL -o test.zip ${URL}
unzip -q test.zip 'data*'

set +e
timeout 10 ${UNSHIELD} -d extract1 x data1.cab > log1 2>&1
CODE=$?
if [ ${CODE} -ne 1 ]; then
    cat log1 >&2
    echo "unshield should have failed with error 1 but was $CODE" >&2
    exit 2
fi

timeout 10 ${UNSHIELD} -O -d extract2 x data1.cab > log2 2>&1
CODE=$?
if [ ${CODE} -ne 0 ]; then
    cat log2 >&2
    echo "unshield failed with error $CODE" >&2
    exit 3
fi

cd extract2
find . -type f -print0 | LC_ALL=C sort -z | xargs -0 md5sum > ../md5
if ! diff -wu ${MD5_FILE} ../md5 >&2 ; then
    echo "MD5 sums diff" >&2
    exit 4
fi

exit 0
