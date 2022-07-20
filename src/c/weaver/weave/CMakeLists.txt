add_library(WeavePass MODULE
    # List your source files here.
    perfflow_weave.cpp
)

# Use C++11 to compile our pass (i.e., supply -std=c++11).
target_compile_features(WeavePass PRIVATE cxx_range_for cxx_auto_type)

# LLVM is (typically) built with no C++ RTTI. We need to match that;
# otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(WeavePass PROPERTIES
    COMPILE_FLAGS "-fno-rtti"
)

find_library(JANSSON_LIB jansson)
target_link_libraries(WeavePass perfflow_parser "${JANSSON_LIB}")

install(TARGETS WeavePass
        EXPORT WeavePass
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
