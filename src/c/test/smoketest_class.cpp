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
#include <unistd.h>

using namespace std;

class SmokeTest
{
public:
    void foo();
};

__attribute__((annotate("@critical_path()")))
void SmokeTest::foo()
{
    usleep(2000000); // Sleep fpr 2 seconds.
    std::cout << "Hello from foo function in SmokeTest class." << std::endl;
}

int main()
{
    SmokeTest st;

    st.foo();

    return 0;
}

