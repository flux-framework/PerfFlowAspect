find_package(Threads REQUIRED)

if(PERFFLOWASPECT_WITH_MULTITHREADS)
    message(STATUS "Building with multi-threading (PERFFLOWASPECT_WITH_MULTITHREADS == ON)")
endif()
