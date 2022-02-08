/*****************************************************************\
 * Copyright 2021-2022 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\*****************************************************************/

#include <cstdio>
#include <string>

void bas ()
{
    printf ("bas");
}

__attribute__((annotate("@critical _path()")))
void bar ()
{
   printf ("bar\n");
   bas ();
}

__attribute__((annotate("@critical_path()")))
int foo (const std::string &str)
{
   printf ("Hello\n");
   bar ();
   if (str == "hello")
       return 1;
   return 0;
}

int main (int argc, char *argv[])
{
    printf ("Inside main\n");
    foo ("hello");
    return 0;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
