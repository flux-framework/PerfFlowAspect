# first Check for CALIPER_DIR
if(NOT caliper_DIR)
    MESSAGE(FATAL_ERROR "Caliper support needs explicit caliper_DIR")
endif()

if(caliper_DIR)
    message(STATUS "PPP ${caliper_DIR}")
    include(cmake/thirdparty/SetupCaliper.cmake)
endif()
