#!/bin/bash

rm -f 2D_test.log

for ipu_no in 1 2 4 8 16
do
    printf "No of IPU: ${ipu_no}\n ================================================== \n" >> 2D_test.log
    ./main --num-ipus ${ipu_no} >> 2D_test.log
done
