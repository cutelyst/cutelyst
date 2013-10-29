# CMakeLists for UDisks2-QT5 lib extra modules

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/udisks2qt5-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/udisks2qt5-config.cmake
  @ONLY
)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/udisks2qt5-config-version.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/udisks2qt5-config-version.cmake
  @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/udisks2qt5-config.cmake
	${CMAKE_CURRENT_BINARY_DIR}/udisks2qt5-config-version.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/udisks2qt5/
)
