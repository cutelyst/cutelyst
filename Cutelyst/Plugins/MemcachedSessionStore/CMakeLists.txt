# SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
# SPDX-License-Identifier: BSD-3-Clause

set(plugin_memcachedsessionstore_SRC
    memcachedsessionstore.cpp
    memcachedsessionstore_p.h
)

set(plugin_memcachedsessionstore_HEADERS
    memcachedsessionstore.h
    MemcachedSessionStore
)

set(target_name Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}MemcachedSessionStore)
add_library(${target_name}
    ${plugin_memcachedsessionstore_SRC}
    ${plugin_memcachedsessionstore_HEADERS}
)
add_library(Cutelyst::MemcachedSessionStore ALIAS ${target_name})

generate_export_header(${target_name} BASE_NAME CUTELYST_PLUGIN_MEMCACHEDSESSIONSTORE)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/cutelyst_plugin_memcachedsessionstore_export.h
    DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins
)

set_target_properties(${target_name} PROPERTIES
    EXPORT_NAME MemcachedSessionStore
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(${target_name}
    PUBLIC Cutelyst::Session
    PRIVATE Cutelyst::Memcached
)

set_property(TARGET ${target_name} PROPERTY PUBLIC_HEADER ${plugin_memcachedsessionstore_HEADERS})
install(TARGETS ${target_name}
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins/MemcachedSessionStore COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQtMemcachedSessionStore.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

if(UNIX)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQtMemcachedSessionStore.5.in
        ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.5
        @ONLY
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.5 DESTINATION ${CMAKE_INSTALL_MANDIR}/man5)
endif()
