# create convenience target that bundles all reg perfflowaspect deps
add_library(perfflowaspect::perfflowaspect INTERFACE IMPORTED)

set_property(TARGET perfflowaspect::perfflowaspect
             PROPERTY INTERFACE_LINK_LIBRARIES
             perfflowaspect)

if(NOT PerfFlowAspect_FIND_QUIETLY)
    message(STATUS "PERFFLOWASPECT_VERSION        = ${PERFFLOWASPECT_VERSION}")
    message(STATUS "PERFFLOWASPECT_INSTALL_PREFIX = ${PERFFLOWASPECT_INSTALL_PREFIX}")

    set(_print_targets "")
    set(_print_targets "perfflowaspect::perfflowaspect ")
    message(STATUS "PerfFlowAspect imported targets: ${_print_targets}")
    unset(_print_targets)
endif()
