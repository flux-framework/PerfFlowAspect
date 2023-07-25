/************************************************************\
 * Copyright 2021 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define N 3

__attribute__((annotate("@critical_path(pointcut='around')")))
void foo(int threadid)
{
    long tid;
    tid = (long)threadid;
    printf("This is worker_thread() %ld \n", tid);
}

int main()
{
    long id;
    #pragma omp
    {
        printf("Nthreads %d\n", omp_get_num_threads());
    }
    #pragma omp parallel for
    for (id = 0; id <= N; id++)
    {
        int thread_id = omp_get_thread_num();
        foo(thread_id);
    }
}
