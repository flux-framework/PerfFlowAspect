###############################################################################
# Example that shows how to use an installed instance of PerfFlowAspect in 
# another CMake-based build system.
#
# To build:
#   mkdir build
#   cd build
#   cmake -DPERFFLOWASPECT_DIR={perfflowaspect install path} ../
#   make
#   ./smoketest
#
# If run in-sub directory below using-with-cmake in a PerfFlowAspect install,
# PERFFLOWASPECT_DIR will be defaulted to ../../..
#
#   mkdir build
#   cd build
#   cmake ..
#   make
#   ./smoketest
###############################################################################

cmake_minimum_required(VERSION 3.0)

project(using_with_cmake)

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

#
# Provide default for PERFFLOWASPECT_DIR that works for an PerfFlowAspect install
#
if(NOT PERFFLOWASPECT_DIR)
    set(PERFFLOWASPECT_DIR "../..")
endif()

#
# Check for valid PerfFlowAspect install
#
get_filename_component(PERFFLOWASPECT_DIR ${PERFFLOWASPECT_DIR} ABSOLUTE)
if(NOT EXISTS ${PERFFLOWASPECT_DIR}/share/perfflowaspect-config.cmake)
    message(FATAL_ERROR "Could not find PerfFlowAspect CMake include file (${PERFFLOWASPECT_DIR}/share/perfflowaspect-config.cmake)")
endif()

#
# Use CMake's find_package to import PerfFlowAspect's targets
#
find_package(PerfFlowAspect REQUIRED
             NO_DEFAULT_PATH
             PATHS ${PERFFLOWASPECT_DIR}/share)

# create our example
add_executable(smoketest smoketest.cpp)

# link to PerfFlowAspect
target_link_libraries(smoketest perfflowaspect::perfflowaspect)

#
