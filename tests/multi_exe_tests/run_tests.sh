#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PROJECT_BASE="$SCRIPT_DIR/../.."

echo "Project Base: $PROJECT_BASE"

if [ -f $PROJECT_BASE/bin/tester ]; then
  echo "Tester binary found"
else
  echo "Building the Tester"
  
  # make directories
  mkdir -p "$PROJECT_BASE/build"
  cd "$PROJECT_BASE/build"
  
  # cmake
  cmake $PROJECT_BASE
  make
  cd -
fi

# we need to be in the test dir to run tests
cd $SCRIPT_DIR

$PROJECT_BASE/bin/tester $SCRIPT_DIR/GradeConfig.json --grade $SCRIPT_DIR/grades.json

status=$?

if [ $status -eq 0 ]; then
  echo "Passed tests with status: $status"
  if [ -e $SCRIPT_DIR/grades.json ]; then
    echo "Grade JSON successfuly created"
    exit 0 
  else
    echo "Failed to output grade JSON"
    exit 1
  fi
else
  echo "Failed tester with exit status: $status"
fi

exit 1
