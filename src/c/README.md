## PerfFlowAspect C/C++  Support

## Overview
Unlike dynamic programming languages such as Python,
C/C++ cannot natively and easily weave PerfFlowAspect
into the target program (e.g., using language
features like Python decorator).
To overcome this language-level limitation,
PerfFlowAspect introduces a simple
[LLVM pass](https://llvm.org/docs/WritingAnLLVMPass.html#introduction-what-is-a-pass)
which can detect critical-path annotations
in the target C/C++ source files and to weave
these annotated program points with
PerfFlowAspect's runtime library as part
of Clang compilers' optimization pass.
This allows C/C++ and Python users alike to use the
consistent aspect-oriented programming (AOP) abstraction
of PerfFlowAspect.
The consistent abstraction and
tool usage flow for both dynamic languages and
more traditional high performance computing
programming languages like C/C++ significantly
facilitate the high level of unity and modularity
as required by the AOP paradigm.

## PerfFlowAspect Dependencies
PerfFlowAspect requires Clang
and LLVM development packages as well as a
jansson-devel package for JSON manipulation.
It additionally requires the dependencies of
our annotation parser code: i.e.,
`flex` and `bison`.
Note that LLVM_DIR must be set to the corresponding
LLVM cmake directory which may differ across
different Linux distributions.

**redhat**                | **ubuntu**              | **version**
----------                | ----------              | -----------
clang                     | clang                   | >= 9.0
llvm-devel                | llvm-dev                | >= 9.0
jansson-devel             | libjansson-dev          | >= 2.6
openssl-devel             | libssl-dev              | >= 1.0.2
cmake                     | cmake                   | >= 3.10
flex                      | flex                    | >= 2.5.37
bison                     | bison                   | >= 3.0.4
make                      | make                    | >= 3.82

##### Installing RedHat/CentOS Packages
```
yum install clang llvm-devel gcc gcc-c++ jansson-devel openssl-devel make make flex bison
```

##### Installing Ubuntu Packages
```
apt-get update
apt install clang llvm-dev libjansson-dev libssl-dev bison flex make cmake make flex bison
```

##### Building PerfFlowAspect
```console
$ module load clang/10.0.1-gcc-8.3.1 (on LLNL systems only)
$ cd PerfFlowAspect/src/c
$ mkdir build && cd build
$ cmake -DCMAKE_CXX_COMPILER=clang++ ../
$ make (note: parallel make (make -j) not supported yet) 

$ ls build/weaver/weave/libWeavePass.so
build/weaver/weave/libWeavePass.so
```

##### Quick Test
Say your target C++ code is `main.cpp`:

```c++
#include <string>
#include <unistd.h>

__attribute__((annotate("@critical_path()")))
void bas ()
{
}

__attribute__((annotate("@critical_path()")))
void bar ()
{
   bas ();
}

__attribute__((annotate("@critical_path()")))
int foo (const std::string &str)
{
   sleep (1);
   bar ();
   return 0;
}

int main (int argc, char *argv[])
{
    foo ("hello");
    return 0;
}
```

Upon successful build and run, a PerfFlowAspect output file will be
generated in the same way as our Python support.
