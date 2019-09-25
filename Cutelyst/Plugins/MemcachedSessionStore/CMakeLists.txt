set(plugin_memcachedsessionstore_SRC
    memcachedsessionstore.cpp
    memcachedsessionstore_p.h
)

set(plugin_memcachedsessionstore_HEADERS
    memcachedsessionstore.h
    MemcachedSessionStore
)

add_library(Cutelyst2Qt5MemcachedSessionStore
    ${plugin_memcachedsessionstore_SRC}
    ${plugin_memcachedsessionstore_HEADERS}
)
add_library(Cutelyst2Qt5::MemcachedSessionStore ALIAS Cutelyst2Qt5MemcachedSessionStore)

set_target_properties(Cutelyst2Qt5MemcachedSessionStore PROPERTIES
    EXPORT_NAME MemcachedSessionStore
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst2Qt5MemcachedSessionStore
    PRIVATE Cutelyst2Qt5::Core
    PRIVATE Cutelyst2Qt5::Session
    PRIVATE Cutelyst2Qt5::Memcached
)

set_property(TARGET Cutelyst2Qt5MemcachedSessionStore PROPERTY PUBLIC_HEADER ${plugin_memcachedsessionstore_HEADERS})
install(TARGETS Cutelyst2Qt5MemcachedSessionStore
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION bin COMPONENT runtime
    ARCHIVE DESTINATION lib COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst2-qt5/Cutelyst/Plugins/MemcachedSessionStore COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5MemcachedSessionStore.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5MemcachedSessionStore.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5MemcachedSessionStore.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

if(UNIX)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5MemcachedSessionStore.5.in
        ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5MemcachedSessionStore.5
        @ONLY
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5MemcachedSessionStore.5 DESTINATION ${MANDIR}/man5)
endif()
