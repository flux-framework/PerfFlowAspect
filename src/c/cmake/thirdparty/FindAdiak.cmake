set(adiak_DIR "/g/g14/greene36/Adiak/install/lib/cmake/adiak")

find_package(adiak REQUIRED)

message(STATUS "Building Adiak smoketest (PERFFLOWASPECT_WITH_ADIAK == ON)")

if(adiak_FOUND)
    message(STATUS "Adiak found: ${adiak_FOUND}")
    message(STATUS "Adiak include directories: ${adiak_INCLUDE_DIRS}")
    message(STATUS "Adiak library directories: ${adiak_LIBRARY_DIRS}")
    message(STATUS "Adiak libraries: ${adiak_LIBRARIES}")
else()
    message(FATAL_ERROR "Adiak not found.")
endif()

include_directories(${adiak_INCLUDE_DIRS})
link_directories(${adiak_LIBRARY_DIRS})