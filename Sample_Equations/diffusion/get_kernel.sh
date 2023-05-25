#!/bin/bash

set -e

TARGET_LINE=$(DEVITO_LOGGING=DEBUG python3 diffusion_3D_wBCs.py 2>&1 | grep "Operator \`Kernel\` fetched")
IFS='\`' read -r -a array <<< "${TARGET_LINE}"

# echo -en "${array[3]}\n"
TARGET_FILE=${array[3]}

mkdir -p c_files

cat $TARGET_FILE > ./c_files/kernel.c