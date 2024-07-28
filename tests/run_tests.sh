#!/bin/bash

CWD=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PROJECT_BASE="$CWD/.."

ls $CWD
ls $PROJECT_BASE

TEST_CONFIGS=(
  "$CWD/ConfigSingleExe.json"
  "$CWD/ConfigMultiExe.json"
  "$CWD/ConfigExpectedFail.json"
)

GRADE_SCRIPT="grader.py"

GRADE_JSON="grades.json"

GRADE_CSV="grades.csv"

#========= RUN Single Executable Tests =========#
$PROJECT_BASE/bin/tester ${TEST_CONFIGS[0]} --timeout 10
if [ $? -ne 0 ]; then
  echo "Tester failed test for config: ${TEST_CONDIGS[0]}"
  exit 1
fi

#========= RUN Expected Failure Tests =========#
$PROJECT_BASE/bin/tester ${TEST_CONFIGS[2]} --timeout 10
if [ $? -ne 1 ]; then
  echo "Tester failed test for config: ${TEST_CONDIGS[2]}"
  exit 1
fi

#========= RUN Grader Tests =========#

$PROJECT_BASE/bin/tester ${TEST_CONFIGS[1]} --grade ${GRADE_JSON} --timeout 1

# Assert GRADE_JSON exists
if [[ ! -e "${CWD}/${GRADE_JSON}" ]]; then
    echo "Error: ${GRADE_JSON} does not exist."
    exit 1
fi

exit 0

#### TODO: What is the best way to check if the grader worked as expected?
python "${CWD}/scripts/${GRADE_SCRIPT}" -f "${CWD}/${GRADE_JSON}"

# Assert GRADE_CSV exists here
if [[ ! -e "${CWD}/${GRADE_CSV}" ]]; then
    echo "Error: ${GRADE_CSV} does not exist."
    exit 1
fi


