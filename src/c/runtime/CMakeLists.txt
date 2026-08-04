set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -O0 -g -std=c++11")

set(perfflow_runtime_headers
    advice_base.hpp
    advice_chrome_tracing.hpp
    advice_dispatcher.hpp
)

set(perfflow_runtime_sources
    advice_chrome_tracing.cpp
    advice_dispatcher.cpp
    perfflow_runtime.cpp
)

include_directories(${JANSSON_INCLUDE_DIRS})

if(PERFFLOWASPECT_WITH_ADIAK)
    include_directories(${adiak_INCLUDE_DIR})
endif()
if(PERFFLOWASPECT_WITH_CALIPER)
    include_directories(${caliper_INCLUDE_DIR})
endif()

add_library(perfflow_runtime SHARED
            ${perfflow_runtime_sources}
            ${perfflow_runtime_headers}
)

if (PERFFLOWASPECT_WITH_ADIAK)
    target_link_libraries(perfflow_runtime ${JANSSON_LIBRARY}
                         OpenSSL::SSL OpenSSL::Crypto adiak::adiak)
else()
    target_link_libraries(perfflow_runtime ${JANSSON_LIBRARY}
                         OpenSSL::SSL OpenSSL::Crypto )
endif()
if(PERFFLOWASPECT_WITH_CALIPER)
    target_link_libraries(perfflow_runtime ${JANSSON_LIBRARY} OpenSSL::SSL OpenSSL::Crypto caliper)
else()
    target_link_libraries(perfflow_runtime ${JANSSON_LIBRARY} OpenSSL::SSL OpenSSL::Crypto)
endif()

install(TARGETS perfflow_runtime
        EXPORT perfflow_export
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
