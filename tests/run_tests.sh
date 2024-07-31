#!/bin/bash

CWD=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PROJECT_BASE="$CWD/.."

cd $CWD

TEST_CONFIGS=(
  "$CWD/ConfigSingleExe.json"
  "$CWD/ConfigGrade.json"
  "$CWD/ConfigExpectedFail.json"
  "$CWD/ConfigRuntime.json"
)

# Grading variables
GRADE_SCRIPT="${CWD}/scripts/grader.py"
GRADE_JSON="${CWD}/scripts/grades.json"
GRADE_CSV="${CWD}/scripts/grades.csv"
GRADE_TIMING_CSV="${CWD}/scripts/grades_timed.csv"

# Timed grading variables
TA_PACKAGE="TA"
TIMED_TOOLCHAIN="LLVM-opt"
TIMED_EXE_REFERENCE="TA"
TIMED_PACKAGE="timed_tests"

#========= RUN Grader Tests =========#

# If grade JSON does not exist, creat it.
if [[ ! -e "${GRADE_JSON}" ]]; then
    $PROJECT_BASE/bin/tester ${TEST_CONFIGS[1]} --grade ${GRADE_JSON} --timeout 1
fi

# Assert GRADE_JSON exists after running the tester in grade mode
if [[ ! -e "${GRADE_JSON}" ]]; then
    echo "Script Error: ${GRADE_JSON} does not exist."
    exit 1
fi

#====  RUN Grader script in regular mode ====#
python "${GRADE_SCRIPT}" "${GRADE_JSON}" "-o" "${GRADE_CSV}" "--ta-package" "${TA_PACKAGE}"

# Assert GRADE_CSV exists here
if [[ ! -e "${GRADE_CSV}" ]]; then
    echo "Script Error: ${GRADE_CSV} does not exist."
    exit 1
fi

# #====  RUN Grader script in regular mode ====#
python "${GRADE_SCRIPT}" "${GRADE_JSON}" "-o" "${GRADE_TIMING_CSV}" \
"--ta-package" "${TA_PACKAGE}" \
"--timed-toolchain" "${TIMED_TOOLCHAIN}" \
"--timed-exe-reference" "${TIMED_EXE_REFERENCE}" \
"--timed-package" "${TIMED_PACKAGE}"

# Assert GRADE_CSV exists here
if [[ ! -e "${GRADE_TIMING_CSV}" ]]; then
    echo "Script Error: ${GRADE_TIMING_CSV} does not exist."
    exit 1
fi

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

#========= RUN Runtime Tests =========#
SO_SCRIPT_DIR=$CWD/lib/
cd $SO_SCRIPT_DIR
./make_so.sh
if [ !? -ne 0 ]; then
  echo "Failed to make mock runtime library!"
  exit 1
fi
cd -

$PROJECT_BASE/bin/tester ${TEST_CONFIGS[3]} --timeout 10
if [ $? -ne 0 ]; then
  echo "Tester failed test for config: ${TEST_CONDIGS[3]}"
  exit 1
fi