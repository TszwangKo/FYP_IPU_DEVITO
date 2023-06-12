#!/bin/bash

cd /home/aaronko/workspace/FYP_IPU_DEVITO/IPU/WaveEquation3D/TO2/
mkdir -p BenchMarks
mkdir -p ./devito/BenchMarks


set -e

NT=3000
SIDE=(0 240 295 0 363)
IPU_NO=(1 2 4)
while getopts "itrc" flag
do
    case "${flag}" in
         i) 
            echo -ne "Running BenchMarks on IPU only \n"
            LOAD_EXE="--load-exe";;
         t)
            echo -ne "Running test size problem\n"
            NT=40
            SIDE=(0 31)
            IPU_NO=(1);;
         r)
            rm ./BenchMarks/* ./output/* ./devito/BenchMarks/* ./devito/output/*
            EXIT 0;;
         c) 
            CPU="--cpu";;
    esac
done


for i in "${IPU_NO[@]}"
do
   cd /home/aaronko/workspace/FYP_IPU_DEVITO/IPU/WaveEquation3D/TO2/
   BUFFERED_SIDE=${SIDE[${i}]}
   INNER_SIDE_LENGTH=$((${BUFFERED_SIDE}-20))

   >&2 echo -e "\n==========================================================\nRunnning Benchmark (SIDE=${BUFFERED_SIDE} ; NT=${NT} ; IPUs=${i})\n"
   

   cd /home/aaronko/workspace/FYP_IPU_DEVITO/IPU/WaveEquation3D/TO2/devito
   # if [ -z ${LOAD_EXE+x} ]
   # then
      echo -ne "BenchMarking CPU...\n"
      DEVITO_LANGUAGE=openmp DEVITO_LOGGING=DEBUG python3 wave_eq_3D.py --shape ${INNER_SIDE_LENGTH} ${INNER_SIDE_LENGTH} ${INNER_SIDE_LENGTH} --nt $NT 2> ./BenchMarks/${BUFFERED_SIDE}x${NT}xcpu.txt && >&2 echo -e "\njson created for ${i} ipu side: ${BUFFERED_SIDE}(Optimised)\n"
   # fi
   cd /home/aaronko/workspace/FYP_IPU_DEVITO/IPU/WaveEquation3D/TO2/
   
   echo -ne "BenchMarking IPU...\n"
   ./main --num-ipus ${i} --depth ${BUFFERED_SIDE} --height ${BUFFERED_SIDE} --width ${BUFFERED_SIDE} --nt $NT $LOAD_EXE $CPU > ./BenchMarks/${BUFFERED_SIDE}x${NT}x${i}ipus.txt
   >&2 echo -ne "Benchmark Completed (SIDE=${BUFFERED_SIDE} ; NT=${NT} ; IPUs=${i})\n==========================================================\n"
done