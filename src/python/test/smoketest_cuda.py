#!/usr/bin/env python

import perfflowaspect
import perfflowaspect.aspect

import numba
import numpy

from numba import jit
import numpy as np
# to measure exec time
from timeit import default_timer as timer

print("Numba version:", numba.__version__)
print("Numpy version:", numpy.__version__)


# normal function to run on cpu
def func(a):
    for i in range(10000000):
        a[i] += 1


# function optimized to run on gpu
@perfflowaspect.aspect.critical_path(pointcut="around")
@jit(target='cuda')
def func2(a):
    for i in range(10000000):
        a[i] += 1


if __name__ == "__main__":
    n = 10000000
    a = np.ones(n, dtype=np.float64)

    start = timer()
    func(a)
    print("without GPU:", timer() - start)

    start = timer()
    func2(a)
    print("with GPU:", timer() - start)


# Kushwaha H. (Jan 11, 2023) Running Python script on GPU
# [source code] https://www.geeksforgeeks.org/running-python-script-on-gpu/
