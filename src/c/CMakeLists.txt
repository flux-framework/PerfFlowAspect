cmake_minimum_required(VERSION 3.12)

project(PerfFlowAspect VERSION "0.1.0")

# Fail if someone tries to config an in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   message(FATAL_ERROR "In-source builds are not supported. Please remove "
                       "CMakeCache.txt from the 'src' dir and configure an "
                       "out-of-source build in another directory.")
endif()

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

# Always use position independent code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set build type
set(default_build_type "Debug")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Using default build type: \"${default_build_type}\"")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
else()
  message(STATUS "Setting build type to \"${CMAKE_BUILD_TYPE}\"")
endif()


include(GNUInstallDirs)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
message(STATUS "CMAKE_INSTALL_RPATH = ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")


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

add_library(perfflowaspect INTERFACE)
target_link_libraries(perfflowaspect INTERFACE perfflow_runtime perfflow_parser WeavePassPlugin)
install(TARGETS perfflowaspect
        EXPORT perfflow_export
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} 
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} 
        RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR} 
)

add_subdirectory(config)

message(STATUS "Config Dir: ${PERFFLOWASPECT_INSTALL_CONFIG_DIR}")

if (NOT DEFINED PERFFLOWASPECT_INSTALL_CONFIG_DIR)
    set(PERFFLOWASPECT_INSTALL_CONFIG_DIR "share")
endif()

install(EXPORT perfflow_export
    FILE perfflowaspect_targets.cmake
    NAMESPACE perfflowaspect::
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${PERFFLOWASPECT_INSTALL_CONFIG_DIR}
)

message(STATUS "PerfFlowAspect version: \"${PROJECT_VERSION}\"")
