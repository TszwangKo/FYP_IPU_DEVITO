#!/bin/bash


side(){
    if [ $1 == 1 ]; then
        SIDE=338
    elif [ $1 == 2 ]; then
        SIDE=421
    elif [ $1 == 4 ]; then
        SIDE=511
    elif [ $1 == 8 ]; then
        SIDE=643
    elif [ $1 == 16 ]; then
        SIDE=810
    else
        SIDE=0
        echo -ne "$SIDE not supported\n"
    fi
}

ITERATION=1000

compile_only () {
   BUFFERED_SIDE=$(($SIDE+4))
   >&2 echo -e "\n==========================================================\nCompiling (SIDE=${BUFFERED_SIDE} ; ITER=${ITERATION} ; IPUs=${i})\n"
   ./main --num-ipus ${i} --depth ${BUFFERED_SIDE} --height ${BUFFERED_SIDE} --width ${BUFFERED_SIDE} --num-iterations $ITERATION --compile-only >> BenchMarks/log.txt
   >&2 echo -e "Compile Complete (SIDE=${BUFFERED_SIDE} ; ITER=${ITERATION} ; IPUs=${i})\n==========================================================\n"
}
mkdir -p
touch log/log.txt
for i in 1 2 4 8 16 
do
    side $i
    compile_only
done