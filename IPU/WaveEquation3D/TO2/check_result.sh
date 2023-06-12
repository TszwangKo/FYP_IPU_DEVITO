#!/bin/bash

set -e

make
./main --vertex "WaveEquationOptimised" > optimised.log 

diff key.json key_comp.json
if [[ $? != 0 ]]; then
  echo "result changed :(, check implementation"
  exit 1
fi

diff optimised.log unoptimised.log