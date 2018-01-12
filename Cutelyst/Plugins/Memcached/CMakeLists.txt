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

add_library(Cutelyst2Qt5Memcached SHARED
    ${plugin_memcached_SRC}
    ${plugin_memcached_HEADERS}
)
add_library(Cutelyst2Qt5::Memcached ALIAS Cutelyst2Qt5Memcached)

set_target_properties(Cutelyst2Qt5Memcached PROPERTIES
    EXPORT_NAME Memcached
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst2Qt5Memcached
    PRIVATE Cutelyst2Qt5::Core
    ${MEMCACHED_LIBRARIES}
)

set_property(TARGET Cutelyst2Qt5Memcached PROPERTY PUBLIC_HEADER ${plugin_memcached_HEADERS})
install(TARGETS Cutelyst2Qt5Memcached
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION include/cutelyst2-qt5/Cutelyst/Plugins/Memcached
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5Memcached.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5Memcached.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5Memcached.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

if(UNIX)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5Memcached.5.in
        ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5Memcached.5
        @ONLY
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5Memcached.5 DESTINATION ${MANDIR}/man5)
endif()
