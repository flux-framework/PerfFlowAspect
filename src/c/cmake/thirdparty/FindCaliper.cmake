## most common case: caliper is built with adiak support
## and caliper needs us to find adiak, or else find_pacakge caliper
## will fail
#
## Check for ADIAK_DIR
#
#if(NOT ADIAK_DIR)
#    MESSAGE(FATAL_ERROR "Caliper support needs explicit ADIAK_DIR")
#endif()
#
#message(STATUS "Looking for Adiak in: ${ADIAK_DIR}")
#
#find_package(adiak REQUIRED
#             NO_DEFAULT_PATH
#             PATHS ${ADIAK_DIR}/lib/cmake/adiak)

message(STATUS "Looking for Caliper in: ${caliper_DIR}")

find_package(caliper REQUIRED
             PATHS ${caliper_DIR}/share/cmake/caliper
             NO_DEFAULT_PATH
             NO_CMAKE_ENVIRONMENT_PATH
             NO_CMAKE_PATH
             NO_SYSTEM_ENVIRONMENT_PATH
             NO_CMAKE_SYSTEM_PATH)

message(STATUS "FOUND Caliper: ${caliper_INSTALL_PREFIX}")

#set(ADIAK_FOUND TRUE)
set(CALIPER_FOUND TRUE)
set(PERFFLOWASPECT_CALIPER_ENABLED TRUE)
