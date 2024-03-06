set(SMOKETESTS
    smoketest
    smoketest2
    smoketest3
    smoketest_class
    smoketest_MPI
    smoketest_MT
)

if(MPI_FOUND)
    include_directories(${MPI_INCLUDE_PATH})
endif()

set(perfflow_deps "-L../runtime -lperfflow_runtime" OpenSSL::Crypto)
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

if(CUDA_FOUND)
    message(STATUS " [*] Adding test: smoketest_cuda")
    set(CUDA_NVCC_FLAGS  "-ccbin ${CMAKE_CXX_COMPILER} -Xcompiler=-Xclang -Xcompiler=-load -Xcompiler=-Xclang -Xcompiler=../../../weaver/weave/libWeavePass.so")
    cuda_add_executable(smoketest_cuda smoketest_cuda_wrapper.cpp smoketest_cuda_kernel.cu)
    target_link_libraries(smoketest_cuda ${perfflow_deps} ${CUDA_LIBRARIES} cuda)
endif()

configure_file(t0001-cbinding-basic.t.in
    ${CMAKE_CURRENT_BINARY_DIR}/t0001-cbinding-basic.t
    @ONLY)

install(TARGETS ${SMOKETESTS}
        DESTINATION test)

if(CUDA_FOUND)
    install(TARGETS ${SMOKETESTS} smoketest_cuda
            DESTINATION test)
endif()

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/t0001-cbinding-basic.t
        DESTINATION test)
