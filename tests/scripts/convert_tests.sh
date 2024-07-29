#!/bin/bash

# script: copy_tests.sh
# author: justin meimar
# description: convert old test formats from F23 into F24 tester format.
# 

# Get the absolute path of the script's directory
script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

directories=($(find . -maxdepth 1 -type d -exec basename {} \; | tail -n +2))

if [ ! -d $testdir ]; then
    echo "Could not find test directory."
    exit 1
fi

for dir in "${directories[@]}"; do
    # echo "$dir"
    cd "$dir"
    
    convert_dir="${script_dir}/${dir}/tests"

    if [ -d "${script_dir}/${dir}/testfiles" ]; then 
        continue 
    fi
    echo "${convert_dir}"
    python "${script_dir}/convertFiles.py" "${convert_dir}"

    cd ..
done