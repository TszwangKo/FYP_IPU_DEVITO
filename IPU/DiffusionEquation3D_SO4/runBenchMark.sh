#!/bin/bash

mkdir -p BenchMarks

ITERATION=10

i=4
INNER_SIDE_LENGTH=520
STEP=2
RESULT=1

run_benchmark () {
   BUFFERED_SIDE=$(($INNER_SIDE_LENGTH+4))
   >&2 echo -e "\n==========================================================\nRunnning Benchmark (SIDE=${BUFFERED_SIDE} ; ITER=${ITERATION} ; IPUs=${i})\n"
   ./main --num-ipus ${i} --depth ${BUFFERED_SIDE} --height ${BUFFERED_SIDE} --width ${BUFFERED_SIDE} --num-iterations $ITERATION > ./BenchMarks/${BUFFERED_SIDE}x${ITERATION}x${i}ipus.txt
   RESULT=$?
   >&2 echo -e "Benchmark Complete (SIDE=${BUFFERED_SIDE} ; ITER=${ITERATION} ; IPUs=${i})\n==========================================================\n"
}

run_benchmark

while [ ${RESULT} != 0 ]; do
   INNER_SIDE_LENGTH=$((${INNER_SIDE_LENGTH}-${STEP}))
   run_benchmark
done



# SIDE_ARRAY=( 0 338 411 0 515 )
# for i in  2 
# do
   
#    cd /home/aaronko/workspace/FYP_IPU_DEVITO/IPU/DiffusionEquation3D_SO4/
#    INNER_SIDE_LENGTH=${SIDE_ARRAY[i]}
#    BUFFERED_SIDE=$(($INNER_SIDE_LENGTH+4))
#    >&2 echo -e "\n==========================================================\nRunnning Benchmark (SIDE=${BUFFERED_SIDE} ; ITER=${ITERATION} ; IPUs=${i})\n"
#    ./main --num-ipus ${i} --depth ${BUFFERED_SIDE} --height ${BUFFERED_SIDE} --width ${BUFFERED_SIDE} --num-iterations $ITERATION > ./BenchMarks/${BUFFERED_SIDE}x${ITERATION}x${i}ipus.txt
#    >&2 echo -e "Benchmark Complete (SIDE=${BUFFERED_SIDE} ; ITER=${ITERATION} ; IPUs=${i})\n==========================================================\n"
# done

 