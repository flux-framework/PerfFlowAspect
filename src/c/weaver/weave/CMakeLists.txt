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

if(CUDA_FOUND)
    enable_language(CUDA)
    add_library(WeavePassCudaPlugin INTERFACE)
    target_compile_options(WeavePassCudaPlugin INTERFACE
    "$<$<BUILD_INTERFACE:$<COMPILE_LANGUAGE:CXX>:-Xclang -load -Xclang $<TARGET_FILE:WeavePass>>>$<$<INSTALL_INTERFACE:$<COMPILE_LANGUAGE:CXX>:-Xclang -load -Xclang $<TARGET_FILE:perfflowaspect::WeavePass>>><$<$<BUILD_INTERFACE:$<COMPILE_LANGUAGE:CUDA>:-ccbin ${CMAKE_CXX_COMPILER} -Xcompiler=-Xclang -Xcompiler=-load -Xcompiler=-Xclang -Xcompiler=$<TARGET_FILE:WeavePass>>><$<$<INSTALL_INTERFACE:$<COMPILE_LANGUAGE:CUDA>:-ccbin ${CMAKE_CXX_COMPILER} -Xcompiler=-Xclang -Xcompiler=-load -Xcompiler=-Xclang -Xcompiler=$<TARGET_FILE:perfflowaspect::WeavePass>>>"
)

#    "SHELL:$<BUILD_INTERFACE:$<COMPILE_LANGUAGE:CXX>:-Xclang -load -Xclang $<TARGET_FILE:WeavePass>>$<INSTALL_INTERFACE:$<COMPILE_LANGUAGE:CXX>:-Xclang -load -Xclang $<TARGET_FILE:perfflowaspect::WeavePass>>$<BUILD_INTERFACE:$<COMPILE_LANGUAGE:CUDA>:-ccbin ${CMAKE_CXX_COMPILER} -Xcompiler=-Xclang -Xcompiler=-load -Xcompiler=-Xclang -Xcompiler=$<TARGET_FILE:WeavePass>>$<INSTALL_INTERFACE:$<COMPILE_LANGUAGE:CUDA>:-ccbin ${CMAKE_CXX_COMPILER} -Xcompiler=-Xclang -Xcompiler=-load -Xcompiler=-Xclang -Xcompiler=$<TARGET_FILE:perfflowaspect::WeavePass>>"


#    target_compile_options(WeavePassCudaPlugin INTERFACE
#    $<$<COMPILE_LANGUAGE:CUDA>:-ccbin ${CMAKE_CXX_COMPILER} -Xcompiler=-Xclang -Xcompiler=-load -Xcompiler=-Xclang -Xcompiler=$<TARGET_FILE:WeavePass>>
#SHELL:$<BUILD_INTERFACE:-ccbin ${CMAKE_CXX_COMPILER} -Xcompiler=-Xclang -Xcompiler=-load -Xcompiler=-Xclang -Xcompiler=$<TARGET_FILE:WeavePass>>$<INSTALL_INTERFACE:-ccbin ${CMAKE_CXX_COMPILER} -Xcompiler=-Xclang -Xcompiler=-load -Xcompiler=-Xclang -Xcompiler=$<TARGET_FILE:perfflowaspect::WeavePass>>>"
 #   )

    install(TARGETS WeavePassCudaPlugin
            EXPORT perfflow_export
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

install(TARGETS WeavePass
        EXPORT perfflow_export
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
