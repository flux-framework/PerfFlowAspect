message(STATUS "Looking for Caliper in: ${caliper_DIR}")

find_package(caliper REQUIRED
             PATHS ${caliper_DIR}/share/cmake/caliper
             NO_DEFAULT_PATH
             NO_CMAKE_ENVIRONMENT_PATH
             NO_CMAKE_PATH
             NO_SYSTEM_ENVIRONMENT_PATH
             NO_CMAKE_SYSTEM_PATH)

message(STATUS "FOUND Caliper: ${caliper_DIR}")

#set(ADIAK_FOUND TRUE)
set(CALIPER_FOUND TRUE)
set(PERFFLOWASPECT_CALIPER_ENABLED TRUE)
