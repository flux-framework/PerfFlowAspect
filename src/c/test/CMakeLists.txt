set(SMOKETESTS
    smoketest
    smoketest2
    smoketest3
    smoketest_class
)

set(perfflow_deps "-L../runtime -lperfflow_runtime" OpenSSL::Crypto)

message(STATUS "Adding CXX unit tests")

if(PERFFLOWASPECT_WITH_ADIAK)
    message(STATUS "Including Adiak in test dependencies")
    set(perfflow_deps "${perfflow_deps}" adiak::adiak)
endif()

foreach(TEST ${SMOKETESTS})
    message(STATUS " [*] Adding test: ${TEST}")
    add_executable(${TEST} ${TEST}.cpp)
	set_source_files_properties(${TEST}.cpp COMPILE_FLAGS "-Xclang -load -Xclang ../weaver/weave/libWeavePass.so -fpass-plugin=../weaver/weave/libWeavePass.so -fPIC")
    target_link_libraries(${TEST} ${perfflow_deps})
    message(STATUS "${perfflow_deps}")
endforeach()

# Build Options
option(PERFFLOWASPECT_WITH_MULTITHREADS "Build multi-threaded smoketest" ON)
find_package(CUDA QUIET)
if(CUDA_FOUND)
    option(PERFFLOWASPECT_WITH_CUDA "Build CUDA smoketest" ON)
else()
    option(PERFFLOWASPECT_WITH_CUDA "Build CUDA smoketest" OFF)
endif()
find_package(MPI QUIET)
if(MPI_FOUND)
    option(PERFFLOWASPECT_WITH_MPI "Build MPI smoketest" ON)
else()
    option(PERFFLOWASPECT_WITH_MPI "Build MPI smoketest" OFF)
endif()



if(PERFFLOWASPECT_WITH_MULTITHREADS)
    message(STATUS " [*] Adding test: smoketest_MT")
    add_executable(smoketest_MT smoketest_MT.cpp)
	set_source_files_properties(smoketest_MT.cpp COMPILE_FLAGS "-Xclang -load -Xclang ../weaver/weave/libWeavePass.so -fpass-plugin=../weaver/weave/libWeavePass.so  -fPIC")
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    target_link_libraries(smoketest_MT ${perfflow_deps} pthread)
endif()

if(PERFFLOWASPECT_WITH_MPI)
    message(STATUS " [*] Adding test: smoketest_MPI")
    add_executable(smoketest_MPI smoketest_MPI.cpp)
	set_source_files_properties(smoketest_MPI.cpp COMPILE_FLAGS "-Xclang -load -Xclang ../weaver/weave/libWeavePass.so -fpass-plugin=../weaver/weave/libWeavePass.so  -fPIC")
    include_directories(${MPI_INCLUDE_PATH})
    target_link_libraries(smoketest_MPI ${perfflow_deps} ${MPI_LIBRARIES})
endif()

if(PERFFLOWASPECT_WITH_CUDA)
    message(STATUS " [*] Adding test: smoketest_cuda")
    set(CUDA_NVCC_FLAGS  "-ccbin ${CMAKE_CXX_COMPILER} -Xcompiler=-Xclang -Xcompiler=-load -Xcompiler=-Xclang -Xcompiler=../../../weaver/weave/libWeavePass.so -Xcompiler=-fpass-plugin=../../../weaver/weave/libWeavePass.so")
    cuda_add_executable(smoketest_cuda
        smoketest_cuda_wrapper.cpp
        smoketest_cuda_kernel.cu
    )
    set_source_files_properties(smoketest_cuda_wrapper.cpp COMPILE_FLAGS "-Xclang -load -Xclang ${CMAKE_BINARY_DIR}/weaver/weave/libWeavePass.so -fpass-plugin=${CMAKE_BINARY_DIR}/weaver/weave/libWeavePass.so  -fPIC")
    target_link_libraries(smoketest_cuda
        ${perfflow_deps}
        ${CUDA_LIBRARIES}
    )
endif()

configure_file(t0001-cbinding-basic.t.in
    ${CMAKE_CURRENT_BINARY_DIR}/t0001-cbinding-basic.t
    @ONLY)

install(TARGETS ${SMOKETESTS}
        DESTINATION test)

if(PERFFLOWASPECT_WITH_MULTITHREADS)
    install(TARGETS smoketest_MT
            DESTINATION test)
endif()

if(PERFFLOWASPECT_WITH_MPI)
    install(TARGETS smoketest_MPI
            DESTINATION test)
endif()

if(PERFFLOWASPECT_WITH_CUDA)
    install(TARGETS smoketest_cuda
            DESTINATION test)
endif()

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/t0001-cbinding-basic.t
        DESTINATION test)
