#!/bin/bash


side(){
    if [ $1 == 1 ]; then
        SIDE=340
    elif [ $1 == 2 ]; then
        SIDE=420
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
   RESULT=$?
   if [ "$RESULT" != "0" ]; then
       echo "returned $RESULT"
       return -1
   else
       >&2 echo -e "Compile Complete (SIDE=${BUFFERED_SIDE} ; ITER=${ITERATION} ; IPUs=${i})\n==========================================================\n"
   fi
}
mkdir -p log
touch log/log.txt
for i in 1  4 
do
    side $i
    compile_only
    while [ $? != "0" ]; do
        echo -ne "ipu${i}: $SIDE out of memory exception, retrying with $(( $SIDE-2 ))\n"
        SIDE=$(( $SIDE-2 ))
        sleep 10
        compile_only
    done
done