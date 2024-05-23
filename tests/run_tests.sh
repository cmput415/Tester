#!/bin/bash

PROJECT_BASE="../"

if [ -f $PROJECT_BASE/bin/tester ]; then
  echo "Tester binary found"
else
  echo "Building the Tester"
  mkdir -p "$PROJECT_BASE/build"
  cd "$PROJECT_BASE/build"
  cmake ..
  make
  cd -
fi

$PROJECT_BASE/bin/tester ./TestConfig.json

status=$?

if [ $status -eq 0 ]; then
  echo "Passed tests with status: $status"
else
  echo "Failed tester with exit status: $status"
fi

exit $status
