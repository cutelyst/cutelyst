# CMakeLists for Cutelyst lib extra modules

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cutelystqt5-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cutelystqt5-config.cmake
  @ONLY
)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cutelystqt5-config-version.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cutelystqt5-config-version.cmake
  @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cutelystqt5-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/cutelystqt5-config-version.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cutelystqt5/
)

install(EXPORT CutelystQt5Targets
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/cutelystqt5/"
        FILE CutelystQt5Targets.cmake
        NAMESPACE Cutelyst::
        COMPONENT Devel)
