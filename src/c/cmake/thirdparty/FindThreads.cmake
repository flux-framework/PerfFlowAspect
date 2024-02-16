find_package(Threads REQUIRED)

if(PERFFLOWASPECT_WITH_MULTITHREADS)
    message(STATUS "Building multi-threaded smoketest (PERFFLOWASPECT_WITH_MULTITHREADS == ON)")
endif()
