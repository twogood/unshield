#!/bin/bash
find `dirname $0`/test/v* -name '*.sh' | while read SCRIPT; do
  echo -n "Running test $SCRIPT..."
  bash ${SCRIPT} && echo "succeeded" || echo "FAILED with code $?"
done
