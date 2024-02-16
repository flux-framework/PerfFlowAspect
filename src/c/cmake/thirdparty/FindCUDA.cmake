find_package(CUDA)

if(PERFFLOWASPECT_WITH_CUDA)
    message(STATUS "Building CUDA smoketest (PERFFLOWASPECT_WITH_CUDA == ON)")
endif()
