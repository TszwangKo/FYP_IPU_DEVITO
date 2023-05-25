#!/bin/bash

mkdir -p BenchMarks

set -e

for SIDE in 340 424 529
do
    INNER_SIDE=$((${SIDE}-4))
    >&2 echo -e "\n==========================================================\nRunnning Benchmark side_length: ${INNER_SIDE}(cpu)\n"
    DEVITO_LOGGING=DEBUG python3 diffusion_3D_wBCs.py --shape ${INNER_SIDE} ${INNER_SIDE} ${INNER_SIDE} &> ./BenchMarks/cpu${INNER_SIDE}.txt
    >&2 echo -e "Completed Benchmark for side_length: ${INNER_SIDE} (cpu)\n==========================================================\n"
done

 