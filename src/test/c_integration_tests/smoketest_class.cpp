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
private:
    void bar();
public:
    void foo();
};

__attribute__((annotate("@critical_path()")))
void SmokeTest::bar()
{
    std::cout << "Hello from the bar private method in SmokeTest class." <<
              std::endl;
    usleep(1000000); // Sleep for 1 second.
}

__attribute__((annotate("@critical_path()")))
void SmokeTest::foo()
{
    std::cout << "Hello from foo public method in SmokeTest class." << std::endl;
    usleep(1000000); // Sleep for 1 second.
    bar();
}

int main()
{
    SmokeTest st;

    st.foo();

    return 0;
}

