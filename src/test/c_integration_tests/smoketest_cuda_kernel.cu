/************************************************************\
 * Copyright 2021 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#include <iostream>
#include <math.h>
#include "smoketest_cuda_kernel.cuh"

__attribute__((annotate("@critical_path(pointcut='around')")))
__global__ void add(int n, float *x, float *y)
{
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = blockDim.x * gridDim.x;
    for (int i = index; i < n; i += stride)
    {
        y[i] = x[i] + y[i];
    }
}

namespace Wrapper
{
    __attribute__((annotate("@critical_path(pointcut='around')")))
    void wrapper(void)
    {
        int N = 1 << 20;
        float *x, *y;

        // Allocate Unified Memory – accessible from CPU or GPU
        cudaMallocManaged(&x, N * sizeof(float));
        cudaMallocManaged(&y, N * sizeof(float));

        // initialize x and y arrays on the host
        for (int i = 0; i < N; i++)
        {
            x[i] = 1.0f;
            y[i] = 2.0f;
        }

        // Run kernel on 1M elements on the GPU
        int blockSize = 256;
        int numBlocks = (N + blockSize - 1) / blockSize;

        add <<< numBlocks, blockSize>>>(N, x, y);

        // Wait for GPU to finish before accessing on host
        cudaDeviceSynchronize();

        // Check for errors (all values should be 3.0f)
        float maxError = 0.0f;
        for (int i = 0; i < N; i++)
        {
            maxError = fmax(maxError, fabs(y[i] - 3.0f));
        }
        std::cout << "Max error: " << maxError << std::endl;

        // Free memory
        cudaFree(x);
        cudaFree(y);
    }
}

/*
 * Harris M. (Jan 25, 2017) An Even Easier Introduction to CUDA source code
 * [source code] https://developer.nvidia.com/blog/even-easier-introduction-cuda/ 
 */