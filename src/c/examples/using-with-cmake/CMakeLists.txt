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
