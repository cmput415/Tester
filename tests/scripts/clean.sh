#!/bin/bash

# script: clean.sh
# author: justin meimar
# description: clean each of the cloned student repositories

directories=($(find . -maxdepth 1 -type d -exec basename {} \; | tail -n +2))

for dir in "${directories[@]}"; do

    if [ -d "$dir" ]; then 
        cd $dir
        echo "Cleaning ${dir}"

        # remove build directory
        if [ -d "build" ]; then
            rm -r "./build"
            echo "  -- rm build"
        fi

        # remove generated antlr4 files
        if [ -d "gen" ]; then
            rm -r "./gen"
            echo "  -- rm gen"
        fi

        # remove binary dir
        if [ -d "bin" ]; then
            rm -r "./bin"
            echo "  -- rm bin"
        fi

        if [ -d "grammar/.antlr" ]; then
            rm -r "./grammar/.antlr"
            echo "  -- rm .antlr"
        fi

        cd - > /dev/null
    fi
done