# support C++14 features used by LLVM 10.0.0
set(CMAKE_CXX_STANDARD 17)

if(LLVM_FOUND)
    add_definitions(${LLVM_DEFINITIONS})
    include_directories(${LLVM_INCLUDE_DIRS})
    link_directories(${LLVM_LIBRARY_DIRS})
endif()

add_subdirectory(weave)
