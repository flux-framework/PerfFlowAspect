cmake_minimum_required(VERSION 3.12)

project(PerfFlowAspect VERSION "0.1.0")

# Fail if using Clang < 18.0
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 18.0)
        message(FATAL_ERROR "Clang++ version must be at least 18.0!")
    endif()
else()
    message(FATAL_ERROR "Unsupported CXX compiler: please use Clang >= 18.0")
endif()

include(cmake/CMakeBasics.cmake)

include(cmake/Setup3rdParty.cmake)

include(GNUInstallDirs)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

# the RPATH to be used when installing, but only if it's not a system directory
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
     "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
endif("${isSystemDir}" STREQUAL "-1")

add_subdirectory(parser)
add_subdirectory(runtime)
add_subdirectory(weaver)
add_subdirectory(test)
add_subdirectory(examples)

add_library(perfflowaspect INTERFACE)
target_link_libraries(perfflowaspect INTERFACE perfflow_runtime perfflow_parser WeavePassPlugin)
install(TARGETS perfflowaspect
        EXPORT perfflow_export
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

add_subdirectory(config)

if(NOT DEFINED PERFFLOWASPECT_INSTALL_CONFIG_DIR)
    set(PERFFLOWASPECT_INSTALL_CONFIG_DIR "share")
endif()

message(STATUS "CMAKE_INSTALL_RPATH = ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
message(STATUS "Config Dir: ${CMAKE_INSTALL_PREFIX}/${PERFFLOWASPECT_INSTALL_CONFIG_DIR}")

install(EXPORT perfflow_export
    FILE perfflowaspect_targets.cmake
    NAMESPACE perfflowaspect::
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${PERFFLOWASPECT_INSTALL_CONFIG_DIR}
)

message(STATUS "PerfFlowAspect version: \"${PROJECT_VERSION}\"")
