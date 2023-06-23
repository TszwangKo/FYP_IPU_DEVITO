#!/bin/bash

mkdir -p BenchMarks

SIDE_ARRAY=( 0 360 403 0 508 )
ITERATION=1000
for i in  1 2 4
do
   SIDE=${SIDE_ARRAY[i]}
   >&2 echo -e "\n==========================================================\nRunnning Benchmark (SIDE=${SIDE} ; ITER=${ITERATION} ; IPUs=${i})\n"
   ./main --num-ipus ${i} --depth ${SIDE} --height ${SIDE} --width ${SIDE} --num-iterations $ITERATION > ./BenchMarks/${SIDE}x${ITERATION}x${i}ipus.txt
   >&2 echo -e "Benchmark Complete (SIDE=${SIDE} ; ITER=${ITERATION} ; IPUs=${i})\n==========================================================\n"
done
