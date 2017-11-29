find_package(PkgConfig REQUIRED)
pkg_search_module(MEMCACHED REQUIRED libmemcached)

set(plugin_memcached_SRC
    memcached.cpp
    memcached_p.h
)

set(plugin_memcached_HEADERS
    memcached.h
    Memcached
)

add_library(cutelyst_qt5_plugin_memcached SHARED
    ${plugin_memcached_SRC}
    ${plugin_memcached_HEADERS}
)
add_library(CutelystQt5::Memcached ALIAS cutelyst_qt5_plugin_memcached)
set_property(TARGET cutelyst_qt5_plugin_memcached PROPERTY EXPORT_NAME Memcached)

set_target_properties(cutelyst_qt5_plugin_memcached PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(cutelyst_qt5_plugin_memcached
    PRIVATE cutelyst-qt5
    ${MEMCACHED_LIBRARIES}
)

install(TARGETS cutelyst_qt5_plugin_memcached EXPORT CutelystQt5Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${plugin_memcached_HEADERS}
	DESTINATION include/cutelyst-qt5/Cutelyst/Plugins/Memcached
	COMPONENT Memcached
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5Memcached.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/CutelystQt5Memcached.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CutelystQt5Memcached.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
