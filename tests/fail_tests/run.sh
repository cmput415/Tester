#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PROJECT_BASE="$SCRIPT_DIR/../.."

TEST_CONFIGS=(
  "$SCRIPT_DIR/BasicConfig.json"
)

# we need to be in the test dir to run tests
cd $SCRIPT_DIR

all_failed=true

for TEST_CONFIG in "${TEST_CONFIGS[@]}"; do

  $PROJECT_BASE/bin/tester $TEST_CONFIG --timeout 10
  status=$?

  if [ $status -eq 0 ]; then
    all_failed=false
    echo "Tester failed test for config: $TEST_CONFIG" 
  fi
done

# run C tests
if [ "$all_failed" = true ]; then
  exit 0
else
  exit 1
fi
