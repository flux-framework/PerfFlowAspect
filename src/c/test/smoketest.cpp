/************************************************************\
 * Copyright 2021 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#include <cstdio>
#include <string>
#include <unistd.h>

__attribute__((annotate("@critical_path(pointcut='around')")))
void bas()
{
    printf("bas\n");
}

__attribute__((annotate("@critical_path(pointcut='around')")))
void bar()
{
    printf("bar\n");
    usleep(1000);
    bas();
}

__attribute__((annotate("@critical_path()")))
int foo(const std::string &str)
{
    printf("foo\n");
    usleep(1000);
    bar();
    if (str == "hello")
        return 1;
    return 0;
}

int main(int argc, char *argv[])
{
    printf("Inside main\n");
    for (int i = 0; i < 4; i++)
        foo("hello");
    return 0;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
