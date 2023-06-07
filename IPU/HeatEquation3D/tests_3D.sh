#!/bin/bash


side(){
    if [ $1 == 1 ]; then
        SIDE=360
    elif [ $1 == 2 ]; then
        SIDE=403
    elif [ $1 == 4 ]; then
        SIDE=508
    elif [ $1 == 8 ]; then
        SIDE=640
    elif [ $1 == 16 ]; then
        SIDE=806
    else
        SIDE=0
        echo -ne "$SIDE not supported\n"
    fi
}

for ipu_no in 2 4
do
    side $ipu_no
    printf "No of IPU: ${ipu_no}\n ================================================== \n" >> 3D_test.log
    ./main --num-ipus ${ipu_no} --depth $SIDE --height $SIDE --width $SIDE>> 3D_test.log
done
