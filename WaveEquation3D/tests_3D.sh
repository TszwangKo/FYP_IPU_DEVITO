#!/bin/bash

rm -f 3D_test.log

for ipu_no in 1 2 4 8 16
do
    printf "No of IPU: ${ipu_no}\n ================================================== \n" >> 3D_test.log
    ./main --num-ipus ${ipu_no} >> 3D_test.log
done
