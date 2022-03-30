cmake_minimum_required(VERSION 3.1)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

project(PerfFlowAspect)

find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

find_package(Clang REQUIRED)

# support C++14 features used by LLVM 10.0.0
set(CMAKE_CXX_STANDARD 14)

add_subdirectory(parser)
add_subdirectory(runtime)
add_subdirectory(weaver)
add_subdirectory(test)
