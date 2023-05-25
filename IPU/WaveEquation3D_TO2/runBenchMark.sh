#!/bin/bash

mkdir -p BenchMarks

set -e

for i in 1 2 4
do
   cd /home/aaronko/workspace/FYP_IPU_DEVITO/IPU/WaveEquation3D_TO2/
   INNER_SIDE_LENGTH=`python3 ./side.py --num-ipus ${i}`
   BUFFERED_SIDE=$((${INNER_SIDE_LENGTH}+20))
   >&2 echo -e "\n==========================================================\nRunnning Benchmark for ${i} ipus, side_length: ${BUFFERED_SIDE}... (Optimised)\n"
   

   cd /home/aaronko/workspace/FYP_IPU_DEVITO/IPU/WaveEquation3D_TO2/devito
   python3 wave_eq_3D.py --shape ${INNER_SIDE_LENGTH} ${INNER_SIDE_LENGTH} ${INNER_SIDE_LENGTH} && >&2 echo -e "\njson created for ${i} ipu side: ${BUFFERED_SIDE}(Optimised)\n"

   cd /home/aaronko/workspace/FYP_IPU_DEVITO/IPU/WaveEquation3D_TO2/
   
   ./main --num-ipus ${i} --depth ${BUFFERED_SIDE} --height ${BUFFERED_SIDE} --width ${BUFFERED_SIDE} > ./BenchMarks/Optimised_${i}ipus_${BUFFERED_SIDE}.txt
   >&2 echo -ne "Completed Benchmark for ${i} ipu, side_length: ${BUFFERED_SIDE} (Optimised)\n==========================================================\n"
done