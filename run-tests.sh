#!/bin/bash
ALL_RET=0
for SCRIPT in $(find $(dirname $0)/test/v* -name '*.sh'); do
  echo -n "Running test $SCRIPT..."
  bash ${SCRIPT}
  TEST_RET=$?
  if [ "$TEST_RET" = "0" ]; then
    echo "succeeded"
  else
    echo "FAILED with code $TEST_RET"
    ALL_RET=1
  fi
done
exit $ALL_RET
