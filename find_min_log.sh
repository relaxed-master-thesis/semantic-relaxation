#!/bin/bash

# loop over all directories in /data/benchData

minDir=""
minGets=1000000000

for dir in data/benchData/*/; do
    dir=${dir%*/}
    getFile="$dir/combined_get_stamps.txt"
    getCount=$(wc -l < "$getFile")

    if [ ! -f "$getFile" ]; then
        echo "File $getFile does not exist, skipping..."
        rm -rf "$dir"
        continue
    fi

    if [ "$getCount" -eq 0 ]; then
        echo "File $getFile is empty, skipping..."
        rm -rf "$dir"
        continue
    fi

    if [ "$getCount" -lt "$minGets" ]; then
        minGets=$getCount
        minDir=$dir
    fi

    # echo "Dir $dir has $getCount gets"
done

echo "Minimum get count is $minGets in $minDir"