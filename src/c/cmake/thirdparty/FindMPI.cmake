find_package(MPI REQUIRED)

if(PERFFLOWASPECT_WITH_MPI)
    message(STATUS "Building with MPI (PERFFLOWASPECT_WITH_MPI == ON)")
endif()
