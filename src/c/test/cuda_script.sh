#!/bin/bash

clang++ -Xclang -load -Xclang /g/g92/lisan1/applications/PerfFlowAspect/src/c/build/weaver/weave/libWeavePass.so -fPIC -c main.cpp && nvcc -ccbin clang++ -Xcompiler=-Xclang -Xcompiler=-load -Xcompiler=-Xclang -Xcompiler=/g/g92/lisan1/applications/PerfFlowAspect/src/c/build/weaver/weave/libWeavePass.so -c test.cu && clang++ -o test main.o test.o -L/usr/tce/packages/cuda/cuda-10.1.168/lib64/ -lcuda -lcudart -L/g/g92/lisan1/applications/PerfFlowAspect/src/c/build/runtime -lperfflow_runtime -lcrypto
