set(plugin_memcachedsessionstore_SRC
    memcachedsessionstore.cpp
    memcachedsessionstore_p.h
)

set(plugin_memcachedsessionstore_HEADERS
    memcachedsessionstore.h
    MemcachedSessionStore
)

add_library(cutelyst_qt5_plugin_memcachedsessionstore SHARED
    ${plugin_memcachedsessionstore_SRC}
    ${plugin_memcachedsessionstore_HEADERS}
)
add_library(CutelystQt5::MemcachedSessionStore ALIAS cutelyst_qt5_plugin_memcachedsessionstore)
set_property(TARGET cutelyst_qt5_plugin_memcachedsessionstore PROPERTY EXPORT_NAME MemcachedSessionStore)

set_target_properties(cutelyst_qt5_plugin_memcachedsessionstore PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(cutelyst_qt5_plugin_memcachedsessionstore
    PRIVATE cutelyst-qt5
    PRIVATE cutelyst_qt5_plugin_memcached
    PRIVATE cutelyst_qt5_plugin_session
)

install(TARGETS cutelyst_qt5_plugin_memcachedsessionstore EXPORT CutelystQt5Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${plugin_memcachedsessionstore_HEADERS}
    DESTINATION include/cutelyst-qt5/Cutelyst/Plugins/MemcachedSessionStore
    COMPONENT MemcachedSessionStore
)
