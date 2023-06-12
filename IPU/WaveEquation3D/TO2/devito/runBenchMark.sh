#!/bin/bash

mkdir -p BenchMarks

set -e

for SIDE in 240 295 363
do
    INNER_SIDE=$((${SIDE}-20))
    >&2 echo -e "\n==========================================================\nRunnning Benchmark side_length: ${INNER_SIDE}(cpu)\n"
    DEVITO_LOGGING=DEBUG python3 wave_eq_3D.py --shape ${INNER_SIDE} ${INNER_SIDE} ${INNER_SIDE} &> ./BenchMarks/cpu${INNER_SIDE}.txt
    >&2 echo -e "Completed Benchmark for side_length: ${INNER_SIDE} (cpu)\n==========================================================\n"
done

 