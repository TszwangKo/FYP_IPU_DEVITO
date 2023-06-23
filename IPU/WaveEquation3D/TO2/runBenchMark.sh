#!/bin/bash

cd /home/aaronko/workspace/FYP_IPU_DEVITO/IPU/WaveEquation3D/TO2/
mkdir -p BenchMarks
mkdir -p ./devito/BenchMarks


set -e

NT=3000
SIDE=(0 225 280 0 350)
IPU_NO=(1 2 4)

echo -ne "\n"
while getopts "itrcp" flag
do
    case "${flag}" in
         i) 
            echo -ne "Running BenchMarks on IPU only\n"
            LOAD_EXE="--load-exe";;
         t)
            echo -ne "Running test size problem\n"
            NT=40
            SIDE=(0 31)
            IPU_NO=(1);;
         r)
            rm -f ./BenchMarks/* ./output/* ./devito/BenchMarks/* ./devito/output/* ./devito/*.json ./plot/*
            exit 0;;
         c) 
            CPU="--cpu";;
         p)
            echo -ne "Plot enabled\n"
            PLOT="--plot 1";;
    esac
done


for i in "${IPU_NO[@]}"
do
   cd /home/aaronko/workspace/FYP_IPU_DEVITO/IPU/WaveEquation3D/TO2/
   BUFFERED_SIDE=${SIDE[${i}]}
   INNER_SIDE_LENGTH=$((${BUFFERED_SIDE}-24))

   >&2 echo -e "\n==========================================================\nRunnning Benchmark (SIDE=${BUFFERED_SIDE} ; NT=${NT} ; IPUs=${i})\n"
   

   cd /home/aaronko/workspace/FYP_IPU_DEVITO/IPU/WaveEquation3D/TO2/devito
   if [ -z ${LOAD_EXE+x} ]
   then
      echo -ne "BenchMarking CPU...\n"
      DEVITO_LANGUAGE=openmp DEVITO_LOGGING=DEBUG python3 ./wave_source.py --shape ${INNER_SIDE_LENGTH} ${INNER_SIDE_LENGTH} ${INNER_SIDE_LENGTH} --nt $NT $PLOT 2>&1 | tee ./BenchMarks/${BUFFERED_SIDE}x${NT}xcpu.txt && >&2 echo -e "\njson created for ${i} ipu side: ${BUFFERED_SIDE}(Optimised)\n"
   fi
   cd /home/aaronko/workspace/FYP_IPU_DEVITO/IPU/WaveEquation3D/TO2/
   
   echo -ne "BenchMarking IPU...\n"
   echo -ne "Reading from ./devito/parameters_${BUFFERED_SIDE}_${NT}.json\n"
   time ./main --num-ipus ${i} --depth ${BUFFERED_SIDE} --height ${BUFFERED_SIDE} --width ${BUFFERED_SIDE} --nt $NT $LOAD_EXE $CPU | tee ./BenchMarks/${BUFFERED_SIDE}x${NT}x${i}ipus.txt
   if [ ! -z ${PLOT+x} ]
   then
      echo -ne "Plotting IPU result"
      python3 ./plot.py --name "${BUFFERED_SIDE}x${NT}"
   fi
   >&2 echo -ne "Benchmark Completed (SIDE=${BUFFERED_SIDE} ; NT=${NT} ; IPUs=${i})\n==========================================================\n"
done