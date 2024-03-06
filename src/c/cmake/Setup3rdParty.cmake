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
