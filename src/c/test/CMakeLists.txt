set(SMOKETESTS
    smoketest
    smoketest2
    smoketest3
    smoketest_MPI
    smoketest_MT
)

find_package(MPI REQUIRED)
find_package(Threads REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

set(perfflow_deps "-L../runtime -lperfflow_runtime -lcrypto")
set(THREADS_PREFER_PTHREAD_FLAG ON)

message(STATUS "Adding CXX unit tests")
foreach(TEST ${SMOKETESTS})
    message(STATUS " [*] Adding test: ${TEST}")
    add_executable(${TEST} ${TEST}.cpp)
    target_link_libraries(${TEST} ${MPI_LIBRARIES})
    target_link_libraries(${TEST} pthread)
    set_source_files_properties(${TEST}.cpp COMPILE_FLAGS "-Xclang -load -Xclang ../weaver/weave/libWeavePass.so -fPIC")
    target_link_libraries(${TEST} ${perfflow_deps})
endforeach()

configure_file(t0001-cbinding-basic.t.in
    ${CMAKE_CURRENT_BINARY_DIR}/t0001-cbinding-basic.t
    @ONLY)

install(TARGETS ${SMOKETESTS}
        DESTINATION test)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/t0001-cbinding-basic.t
        DESTINATION test)
