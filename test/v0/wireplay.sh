#!/bin/bash
set -e
cd "$(dirname "$0")"
MD5_FILE=$(pwd)/$(basename "$0" .sh).md5
UNSHIELD=${UNSHIELD:-/var/tmp/unshield/bin/unshield}

if [ \! -x "${UNSHIELD}" ]; then
    echo "unshield executable not found at $UNSHIELD" >&2
    exit 1
fi

DIR=$(mktemp -d)
trap 'rm -rf ${DIR}' TERM INT EXIT
cd "${DIR}"

URL="https://www.dropbox.com/s/ac3418ds94j5542/dpcb1197-Wireplay.zip?dl=1"
curl -sSL -o test.zip "${URL}"
unzip -q test.zip

set +e

timeout 10 "${UNSHIELD}" -O -d extract x dpcb1197-Wireplay/DOS/data1.cab > log 2>&1
CODE=$?
if [ ${CODE} -ne 0 ]; then
    cat log >&2
    echo "unshield failed with error $CODE" >&2
    exit 3
fi

cd extract
find . -type f -print0 | LC_ALL=C sort -z | xargs -0 md5sum > ../md5
if ! diff -wu "${MD5_FILE}" ../md5 >&2 ; then
    echo "MD5 sums diff" >&2
    exit 4
fi

exit 0
