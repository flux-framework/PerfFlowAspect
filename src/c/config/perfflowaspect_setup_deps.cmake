include(CMakeFindDependencyMacro)

if (NOT caliper_DIR)
    set(caliper_DIR ${PERFFLOWASPECT_CALIPER_DIR})
endif()

if(caliper_DIR)
    if(NOT PerfFlowAspect_FIND_QUIETLY)
        message(STATUS "PerfFlowAspect was built with Caliper Support")
        message(STATUS "Looking for Caliper at: ${caliper_DIR}/share/cmake/caliper")
    endif()

    # find caliper
    find_package(caliper REQUIRED
                 NO_DEFAULT_PATH
                 PATHS ${caliper_DIR}/share/cmake/caliper)
endif()
