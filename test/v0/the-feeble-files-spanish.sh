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

URL="https://www.dropbox.com/s/1ng0z9kfxc7eb1e/unshield-the-feeble-files-spanish.zip?dl=1"
curl -fsSL -o test.zip ${URL}
unzip -q test.zip 'data*'

set +e
timeout 10 ${UNSHIELD} -O -d extract1 x data1.cab > log2 2>&1
CODE=$?
if [ ${CODE} -ne 0 ]; then
    cat log2 >&2
    echo "unshield failed with error $CODE" >&2
    echo "See https://github.com/twogood/unshield/issues/27" >&2
    exit 2
fi

cd extract1
find . -type f | LC_ALL=C sort | xargs md5sum > ../md5
if ! diff -wu ${MD5_FILE} ../md5 >&2 ; then
    echo "MD5 sums diff" >&2
    echo "See https://github.com/twogood/unshield/issues/27" >&2
    exit 3
fi

exit 0
