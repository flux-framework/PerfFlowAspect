include(cmake/thirdparty/FindBison.cmake)

if(PERFFLOWASPECT_WITH_CUDA)
    include(cmake/thirdparty/FindCUDA.cmake)
endif()

include(cmake/thirdparty/FindFlex.cmake)

include(cmake/thirdparty/FindJansson.cmake)

include(cmake/thirdparty/FindLLVM.cmake)

if(PERFFLOWASPECT_WITH_MPI)
    include(cmake/thirdparty/FindMPI.cmake)
endif()

include(cmake/thirdparty/FindOpenSSL.cmake)

if(PERFFLOWASPECT_WITH_MULTITHREADS)
    include(cmake/thirdparty/FindThreads.cmake)
endif()

if(PERFFLOWASPECT_WITH_ADIAK)
    if(NOT adiak_DIR)
        message(FATAL_ERROR "PFA + Adiak needs explicit adiak_DIR")
    endif()

    if (adiak_DIR)
        message(STATUS "${adiak_DIR}")
        include(cmake/thirdparty/FindAdiak.cmake)
    endif()

    if (ADIAK_FOUND)
        add_definitions(-DPERFFLOWASPECT_WITH_ADIAK)
    endif()
endif()