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

## Builiding PerfFlowAspect WeavePass
PerfFlowAspect WeavePass requires Clang
and LLVM development packages as well as a
jansson-devel package for JSON manipulation.
It also depends on `src/c/parser/libperfflow_parser.so`
so it additionally requires the dependencies of
our annotation parser code: i.e.,
`flex` and `bison`.

**redhat**                | **ubuntu**              | **version**       | **note**
----------                | ----------              | -----------       | --------
clang                     | clang                   | >= 6.0            | *1*
llvm-devel                | llvm-dev                | >= 6.0            | *1*
jansson-devel             | libjansson-dev          | >= 2.6            |
cmake                     | cmake                   | >= 3.10           |
flex                      | flex                    | >= 2.5.37         |
bison                     | bison                   | >= 3.0.4          |
make                      | make                    | >= 3.82           |

*Note 1 - If you use Clang >=9.0, there are two source code changes
you need to make in `src/c/weaver/weave/perfflow_weave.cpp`.
Please see our in-lined NOTE comments.*

##### Installing RedHat/CentOS Packages
```
yum install clang llvm-devel gcc gcc-c++ jansson-devel cmake make flex bison
```

##### Installing Ubuntu Packages

```
apt-get update
apt install clang llvm-dev libjansson-dev bison flex make cmake make flex bison
```

##### Building PerfFlowAspect Annotation Parser and Runtime

```console
$ cd parser
$ make
$ cd ../runtime
$ make
```

##### Building PerfFlowAspect WeavePass LLVM Plugin

The following generates the WeavePass LLVM plugin.
Note that LLVM_DIR must be set to the corresponding
LLVM cmake directory which may differ across
different Linux distributions.

```console
$ cd ../weaver
$ mkdir build
$ cd build
$ LLVM_DIR=/usr/lib/llvm-6.0/lib/cmake/llvm cmake ..
-- The C compiler identification is GNU 7.5.0
-- The CXX compiler identification is Clang 6.0.0
-- Check for working C compiler: /usr/bin/cc
-- Check for working C compiler: /usr/bin/cc -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Detecting C compile features
-- Detecting C compile features - done
-- Check for working CXX compiler: /usr/bin/clang++-6.0
-- Check for working CXX compiler: /usr/bin/clang++-6.0 -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done
-- Generating done
-- Build files have been written to: /usr/src/src/c/weaver/build

$ make
Scanning dependencies of target WeavePass
[ 50%] Building CXX object weave/CMakeFiles/WeavePass.dir/perfflow_weave.cpp.o
[100%] Linking CXX shared module libWeavePass.so
[100%] Built target WeavePass

$ ls weave/libWeavePass.so
weave/libWeavePass.so
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

You can compile and run this test code
with our WeavePass as the following:

```console
$ clang++ -Xclang -load -Xclang weave/libWeavePass.so -L../runtime/ main.cpp -o main -lperfflow_runtime
$ LD_LIBRARY_PATH=../runtime/ ./main
```

Upon successful run, a PerfFlowAspect output file will be
generated in the same way as our Python support.
