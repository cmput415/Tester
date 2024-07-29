#!/bin/bash

# script: build.sh
# author: justin meimar
# description: build each project in the current directory, track
# builds that fail in a file failed_builds.txt 

script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

# Set the partial config path to be absolute
failed_builds="$script_dir/failed_builds.txt"
> "$failed_builds"

directories=($(find . -maxdepth 1 -type d -exec basename {} \; | tail -n +2))

for dir in "${directories[@]}"; do
    if [ -d "$dir" ]; then 
        echo "-- Building project: ${dir}"

        cd "$dir" || {
            echo "$dir: Failed to change directory" >> "$failed_builds"
            continue
        }
        
        # change into build directory 
        mkdir -p build

        # make VCalc
        cd build || {
            echo "$dir: Failed to change to build directory" >> "$failed_builds"
            cd ..
            continue
        }

        if ! cmake ..; then
            echo "$dir: cmake failed" >> "$failed_builds"
            cd ../..
            continue
        fi

        # Run make and handle failure
        if ! make -j 2; then
            echo "$dir: make failed" >> "$failed_builds"
        fi

        # change back to previous directory for next iteration  
        cd ../..
    fi
done

echo "Build process completed. Check $failed_builds for any failed builds."