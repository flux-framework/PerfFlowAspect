/************************************************************\
 * Copyright 2021 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#include "smoketest_cuda_kernel.cuh"

__attribute__((annotate("@critical_path(pointcut='around')")))
int main(int argc, char *argv[])
{
    Wrapper::wrapper();
}