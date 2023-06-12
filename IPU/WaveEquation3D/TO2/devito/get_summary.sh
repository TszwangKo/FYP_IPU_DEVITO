#!/bin/bash


for filename in ./BenchMarks/*.txt; do
    # for ((i=0; i<=3; i++)); do
        echo -ne "${filename}\n"
        cat ${filename} | grep "Global performance:"
    # done
done