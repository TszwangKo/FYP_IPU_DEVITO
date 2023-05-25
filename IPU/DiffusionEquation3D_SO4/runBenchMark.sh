#!/bin/bash

mkdir -p BenchMarks

set -e


# for i in  1 2 4
for INNER_SIDE_LENGTH in 507
do
   i=4
   cd /home/aaronko/workspace/FYP_IPU_DEVITO/IPU/DiffusionEquation3D_SO4/
   # INNER_SIDE_LENGTH=`python3 ./side.py --num-ipus ${i}`
   BUFFERED_SIDE=$(($INNER_SIDE_LENGTH+4))
   >&2 echo -e "\n==========================================================\nRunnning Benchmark for ${i} ipus, side_length: ${BUFFERED_SIDE}... (Optimised)\n"
   
   
   ./main --num-ipus ${i} --depth ${BUFFERED_SIDE} --height ${BUFFERED_SIDE} --width ${BUFFERED_SIDE} > ./BenchMarks/Optimised_${i}ipus_${BUFFERED_SIDE}.txt
   >&2 echo -e "Completed Benchmark for ${i} ipu, side_length: ${BUFFERED_SIDE} (Optimised)\n==========================================================\n"
done

 