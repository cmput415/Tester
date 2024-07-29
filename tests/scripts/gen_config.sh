#!/bin/bash

# script: gen_config.sh
# author: justin meimar
# description: Once every project is built, copy the paths of the binary and
# runtime into a config file. 

# Get the absolute path of the script's directory
script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

# Set the partial config path to be absolute
partial_config="$script_dir/partial_config.json"
> "$partial_config"

directories=($(find . -maxdepth 1 -type d -exec basename {} \; | tail -n +2))

binary="vcalc"
shared_obj="libvcalcrt.so"

# Echo the binary paths into the parital config
echo "{" >> "$partial_config"
echo "\"testedExecutablePaths\": {" >> "$partial_config"
for dir in "${directories[@]}"; do
    full_dir="$script_dir/$dir"
    team_name=${dir#*-}
    if [ -d "$full_dir" ]; then 
        if [ -d "$full_dir/bin" ]; then
            binary_path=$(find "$full_dir/bin" -name "$binary")
            if [ -n "$binary_path" ]; then
                echo "  -- Copying binary from ${dir}"
                echo "  \"${team_name}\": \"$binary_path\"," >> "$partial_config"
            else
                echo " -- ERROR: Could not find binary for project: ${dir}" 
            fi
        fi
    fi
done

# remove trailing comma on last binary entry
sed -i '$ s/,$//' "$partial_config"

echo "-- Copied binary paths into partial config"

echo "}," >> "$partial_config"

# Echo the runtimes into the parital config
echo "\"runtimes\": {" >> "$partial_config"
for dir in "${directories[@]}"; do
    full_dir="$script_dir/$dir"
    team_name=${dir#*-}
    if [ -d "$full_dir" ]; then 
        if [ -d "$full_dir/bin" ]; then

            so_path=$(find "$full_dir/bin" -name "$shared_obj")

            if [ -n "$binary_path" ] && [ -n "$so_path" ]; then
                echo "  -- Copying runtime from ${dir}"
                echo "  \"${team_name}\": \"$so_path\"," >> "$partial_config"
            else
                echo " -- ERROR: Could not find runtime for project: ${dir}" 
            fi
        fi
    fi
done

# remove trailing comma on shared object entry
sed -i '$ s/,$//' "$partial_config"

echo "}" >> "$partial_config"
echo "}" >> "$partial_config"

echo "-- Copied runtime paths into partial config"