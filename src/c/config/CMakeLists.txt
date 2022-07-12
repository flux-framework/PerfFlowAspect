###############################################################################
# export everything for a cmake build to be able to import with find_package
###############################################################################

if (NOT DEFINED PERFFLOWASPECT_INSTALL_CONFIG_DIR)
    set(PERFFLOWASPECT_INSTALL_CONFIG_DIR "share")
endif()

if (NOT DEFINED PERFFLOWASPECT_INSTALL_LIB_DIR)
    set(PERFFLOWASPECT_INSTALL_LIB_DIR "${CMAKE_INSTALL_LIBDIR}")
endif()

if (NOT DEFINED PERFFLOWASPECT_INSTALL_SHARED_RESOURCES_DIR)
    set(PERFFLOWASPECT_INSTALL_SHARED_RESOURCES_DIR "share/perfflowaspect")
endif()

if (NOT DEFINED PERFFLOWASPECT_INSTALL_CMAKE_MODULE_DIR)
    set(PERFFLOWASPECT_INSTALL_CMAKE_MODULE_DIR "${PERFFLOWASPECT_INSTALL_CONFIG_DIR}")
endif()

include(CMakePackageConfigHelpers)

# write version helper
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/perfflowaspect-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# write cmake package config file
configure_package_config_file(
  perfflowaspect-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/perfflowaspect-config.cmake
  INSTALL_DESTINATION ${PERFFLOWASPECT_INSTALL_CONFIG_DIR}
  PATH_VARS
    PERFFLOWASPECT_INSTALL_LIB_DIR
    PERFFLOWASPECT_INSTALL_SHARED_RESOURCES_DIR
    PERFFLOWASPECT_INSTALL_CMAKE_MODULE_DIR
)

# install everything needed
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/perfflowaspect-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/perfflowaspect-config-version.cmake
    perfflowaspect_setup_targets.cmake
    DESTINATION ${PERFFLOWASPECT_INSTALL_CMAKE_MODULE_DIR})

# Create pkg-config .pc file
set(PKG_CONFIG_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib)
set(PKG_CONFIG_LIBS "-L\${libdir} -lperfflow_runtime -L\${libdir} -lperfflow_parser -lssl -lcrypto")

configure_file(
perfflowaspect.pc.in
${CMAKE_CURRENT_BINARY_DIR}/perfflowaspect.pc
@ONLY)

install(FILES ${PROJECT_BINARY_DIR}/config/perfflowaspect.pc
        DESTINATION share)
