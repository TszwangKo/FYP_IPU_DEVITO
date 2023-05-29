#!/bin/bash

mkdir -p BenchMarks

set -e

ITERATION=10

SIDE_ARRAY=( 0 336 )
for i in  1
do
   cd /home/aaronko/workspace/FYP_IPU_DEVITO/IPU/DiffusionEquation3D_SO4/
   INNER_SIDE_LENGTH=${SIDE_ARRAY[i]}
   BUFFERED_SIDE=$(($INNER_SIDE_LENGTH+4))
   >&2 echo -e "\n==========================================================\nRunnning Benchmark (SIDE=${BUFFERED_SIDE} ; ITER=${ITERATION} ; IPUs=${i})\n"
   ./main --num-ipus ${i} --depth ${BUFFERED_SIDE} --height ${BUFFERED_SIDE} --width ${BUFFERED_SIDE} --num-iterations $ITERATION > ./BenchMarks/${BUFFERED_SIDE}x${ITERATION}x${i}ipus.txt
   >&2 echo -e "Benchmark Complete (SIDE=${BUFFERED_SIDE} ; ITER=${ITERATION} ; IPUs=${i})\n==========================================================\n"
done

 