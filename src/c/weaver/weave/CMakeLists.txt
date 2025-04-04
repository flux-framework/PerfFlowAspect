# Add the new pass
add_library(WeavePass MODULE
    # List your source files here.
    perfflow_weave_common.cpp
    perfflow_weave_new_pass.cpp
)

target_compile_features(WeavePass PRIVATE cxx_std_17 cxx_range_for cxx_auto_type)


# LLVM is (typically) built with no C++ RTTI. We need to match that;
# otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(WeavePass PROPERTIES
    COMPILE_FLAGS "-fno-rtti  -stdlib=libstdc++"
)

target_link_libraries(WeavePass perfflow_parser ${JANSSON_LIB})

add_library(WeavePassPlugin INTERFACE)
target_compile_options(WeavePassPlugin INTERFACE
    "SHELL:$<BUILD_INTERFACE:-Xclang -load -Xclang $<TARGET_FILE:WeavePass>>$<INSTALL_INTERFACE:-Xclang -load -Xclang $<TARGET_FILE:perfflowaspect::WeavePass>>"
)

install(TARGETS WeavePassPlugin
        EXPORT perfflow_export
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(TARGETS WeavePass
        EXPORT perfflow_export
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
