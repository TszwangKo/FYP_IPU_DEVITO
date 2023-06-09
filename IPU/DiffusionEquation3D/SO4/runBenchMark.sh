#!/bin/bash

cd /home/aaronko/workspace/FYP_IPU_DEVITO/IPU/DiffusionEquation3D/SO4/

mkdir -p BenchMarks

ITERATION=1000

#  SIDE_ARRAY=( 0 320 400 0 480 ) for Weak Scaling Experiment
#  SIDE_ARRAY=( 0 342 342 0 342 ) for Strong Scaling Experiment
SIDE_ARRAY=( 0 320 400 0 480 )
for i in  1 2 4
do
   INNER_SIDE_LENGTH=${SIDE_ARRAY[i]}
   BUFFERED_SIDE=$(($INNER_SIDE_LENGTH))
   >&2 echo -e "\n==========================================================\nRunnning Benchmark (SIDE=${BUFFERED_SIDE} ; ITER=${ITERATION} ; IPUs=${i})\n"
   ./main --num-ipus ${i} --depth ${BUFFERED_SIDE} --height ${BUFFERED_SIDE} --width ${BUFFERED_SIDE} --num-iterations $ITERATION > ./BenchMarks/${BUFFERED_SIDE}x${ITERATION}x${i}ipus.txt
   >&2 echo -e "Benchmark Complete (SIDE=${BUFFERED_SIDE} ; ITER=${ITERATION} ; IPUs=${i})\n==========================================================\n"
done

 