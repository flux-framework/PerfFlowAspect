find_package(MPI REQUIRED)

if(PERFFLOWASPECT_WITH_MPI)
    message(STATUS "Building MPI smoketest (PERFFLOWASPECT_WITH_MPI == ON)")
endif()
