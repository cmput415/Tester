#!/bin/bash

# script: copy_tests.sh
# author: justin meimar
# description: Copy test files into one large directory for running the grader. 

# Get the absolute path of the script's directory
script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

testdir="testfiles"
rm -r "${script_dir}/${testdir}"

directories=($(find . -maxdepth 1 -type d -exec basename {} \; | tail -n +2))

mkdir -p "$testdir"

if [ ! -d $testdir ]; then
    echo "Could not find test directory."
    exit 1
fi

for dir in "${directories[@]}"; do
    # echo "$dir"
    cd "$dir"

    team_name=${dir#*-}
    expected_test_dir="${script_dir}/${dir}/tests/testfiles/${team_name}"

    if [ -d "${expected_test_dir}" ]; then
        echo "-- Found properly formatted testfiles ${team_name}"
        echo " -- ${expected_test_dir}"
        cp -r "${expected_test_dir}" "${script_dir}/${testdir}/"

        # cp -r "${expected_test_dir}"
    else
        echo "-- Bad test directory naming: ${team_name}"

        # CASE 1: the team did not wrap all their test in a self-named directory 
        # TODO:

        # CASE 2: the team tried to make a self-named directory, but failed.
        # TODO:
    fi
    cd ..
done