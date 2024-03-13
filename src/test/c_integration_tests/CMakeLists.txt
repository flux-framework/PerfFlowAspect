cmake_minimum_required(VERSION 3.12)

project(PerfFlowAspect_C_Integration VERSION "0.1.0")

# Fail if using Clang < 9.0
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # require at least Clang 9.0
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
        message(FATAL_ERROR "Clang++ version must be at least 9.0!")
    elseif (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 11)
        set(PERFFLOWASPECT_CLANG_11_NEWER TRUE CACHE BOOL "using >=clang11")
        add_definitions(-DPERFFLOWASPECT_CLANG_11_NEWER)
    elseif (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11)
        set(PERFFLOWASPECT_CLANG_11_NEWER FALSE CACHE BOOL "using >=clang11")
    endif()
else()
    message(WARNING "Unsupported CXX compiler: please use Clang >= 9.0")
endif()

find_package(OpenSSL REQUIRED)
find_package(MPI REQUIRED)
find_package(perfflowaspect REQUIRED)

include(GNUInstallDirs)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

# the RPATH to be used when installing, but only if it's not a system directory
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
     "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
endif("${isSystemDir}" STREQUAL "-1")


set(SMOKETESTS
    smoketest
    smoketest2
    smoketest3
    smoketest_class
    smoketest_MPI
    smoketest_MT
)

if(MPI_FOUND)
    include_directories(${MPI_INCLUDE_PATH})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)

message(STATUS "Adding CXX unit tests")
foreach(TEST ${SMOKETESTS})
    message(STATUS " [*] Adding test: ${TEST}")
    add_executable(${TEST} ${TEST}.cpp)
    target_link_libraries(${TEST} ${MPI_LIBRARIES})
    target_link_libraries(${TEST} pthread)
    target_link_libraries(${TEST} 
                            OpenSSL::Crypto 
                            perfflowaspect::WeavePassPlugin 
                            perfflowaspect::perfflow_runtime)
endforeach()

# if(CUDA_FOUND)
#     message(STATUS " [*] Adding test: smoketest_cuda")
#     set(CUDA_NVCC_FLAGS  "-ccbin ${CMAKE_CXX_COMPILER} -Xcompiler=-Xclang -Xcompiler=-load -Xcompiler=-Xclang -Xcompiler=../../../weaver/weave/libWeavePass.so")
#     cuda_add_executable(smoketest_cuda smoketest_cuda_wrapper.cpp smoketest_cuda_kernel.cu)
#     target_link_libraries(smoketest_cuda ${perfflow_deps} ${CUDA_LIBRARIES} cuda)
# endif()

configure_file(t0001-cbinding-basic.t.in
    ${CMAKE_CURRENT_BINARY_DIR}/t0001-cbinding-basic.t
    @ONLY)

install(TARGETS ${SMOKETESTS}
        DESTINATION test)

if(CUDA_FOUND)
    install(TARGETS ${SMOKETESTS} smoketest_cuda
            DESTINATION test)
endif()

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/t0001-cbinding-basic.t
        DESTINATION test)
