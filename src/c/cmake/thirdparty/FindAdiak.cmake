message(STATUS "Looking for Adiak in ${adiak_DIR}")
find_package(adiak REQUIRED 
			PATHS ${adiak_DIR}/lib/cmake/adiak
			NO_DEFAULT_PATH
			NO_CMAKE_ENVIRONMENT_PATH
			NO_CMAKE_PATH
			NO_SYSTEM_ENVIRONMENT_PATH
			NO_CMAKE_SYSTEM_PATH)

message(STATUS "FOUND Adiak: ${adiak_DIR}")
set(ADIAK_FOUND TRUE)
set(PERFFLOWASPECT_ADIAK_ENABLED TRUE)
