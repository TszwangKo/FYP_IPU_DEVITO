# Heat Equation 3D

This folder contains the work of Simen Hopnås on benchmarking the 3D Heat Equation. (See https://github.com/simehaa/IPU)

It is included for the result was predoduced on the Graphcore IPU-POD4. Readers are encouraged to reproduce the results to verfy the numbers obtained.


## Result


| Result      | Processor | Vertex                | Time   | Throughput    | Minimal  Bandwidth |
|-------------|-----------|-----------------------|--------|---------------|--------------------|
| Reproduced  | IPU       | HeatEquationSimple    | 0.36 s | 1.03  TFLOPS  | 3.81  TB/s         |
| Reproduced  | IPU       | HeatEquationOptimized | 0.26 s | 1.45  TFLOPS  | 5.36  TB/s         |
| Original    | IPU       | HeatEquationSimple    | 0.43 s | 0.87 TFLOPS   | 3.11 TB/s          |
| Original    | IPU       | HeatEquationOptimized | 0.26 s | 1.44 TFLOPS   | 5.15 TB/s          |


