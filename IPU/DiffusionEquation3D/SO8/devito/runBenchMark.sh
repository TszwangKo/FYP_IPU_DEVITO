#!/bin/bash

mkdir -p BenchMarks

set -e

ITERATION=1000

SIDE_ARRAY=(0 340 424 0 511)
for nthread in 1 2 4
# for SIDE in 340 424 511
do
    SIDE=${SIDE_ARRAY[nthread]};
    INNER_SIDE=$((${SIDE}-4))
    >&2 echo -e "\n==========================================================\nRunnning Benchmark (SIDE=${SIDE} ; ITER=${ITERATION} ; CPUs=${nthread})\n"
    export OMP_THREAD_LIMIT=${nthread}
    DEVITO_LANGUAGE=openmp DEVITO_LOGGING=DEBUG python3 diffusion_3D_wBCs.py --shape ${INNER_SIDE} ${INNER_SIDE} ${INNER_SIDE} --nt ${ITERATION} &> ./BenchMarks/${SIDE}x${ITERATION}x${OMP_THREAD_LIMIT}cpus.txt
    echo -ne "(SIDE=${SIDE} ; ITER=${ITERATION} ; CPUs=${nthread})\n" >> ./BenchMarks/${SIDE}x${ITERATION}x${OMP_THREAD_LIMIT}cpus.txt
    >&2 echo -e "Benchmark Completed (SIDE=${SIDE} ; ITER=${ITERATION} ; CPUs=${nthread})\n==========================================================\n"
done

 