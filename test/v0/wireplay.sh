#!/bin/sh
set -e
cd "$(dirname "$0")"
MD5_FILE=$(pwd)/$(basename "$0" .sh).md5

. ../functions.sh
set_md5sum    # ${MD5SUM}
set_unshield  # ${UNSHIELD}
set_directory ${HOME}/.cache/unshieldtest/wireplay

download_file "https://www.dropbox.com/s/ac3418ds94j5542/dpcb1197-Wireplay.zip?dl=1" test.zip
clean_directory_except test.zip
cleanup_func 'clean_directory_except test.zip'

unzip -o -q test.zip

# ===================================================================

set +e

timeout 10 "${UNSHIELD}" -O -d extract x dpcb1197-Wireplay/DOS/data1.cab > log 2>&1
CODE=$?
if [ ${CODE} -ne 0 ]; then
    cat log >&2
    echo "unshield failed with error $CODE" >&2
    exit 3
fi

cd extract
find . -type f -print0 | LC_ALL=C sort -z | xargs -0 ${MD5SUM} > ../md5
winFixMd5 ../md5
if ! diff -wu "${MD5_FILE}" ../md5 >&2 ; then
    echo "MD5 sums diff" >&2
    exit 4
fi

exit 0
