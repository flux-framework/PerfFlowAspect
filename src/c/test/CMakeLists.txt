set(SMOKETESTS
    smoketest
    smoketest2
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -Xclang -load -Xclang ../weaver/build/weave/libWeavePass.so")
set(perfflow_deps "-L../runtime -lperfflow_runtime -lcrypto")

message(STATUS "Adding CXX unit tests")
foreach(TEST ${SMOKETESTS})
    message(STATUS " [*] Adding test: ${TEST}")
    add_executable(${TEST} ${TEST}.cpp)
    target_link_libraries(${TEST} ${perfflow_deps})
endforeach()

install(TARGETS ${SMOKETESTS}
        DESTINATION tests)
