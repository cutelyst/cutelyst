# SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
# SPDX-License-Identifier: BSD-3-Clause

find_package(PkgConfig REQUIRED)
pkg_search_module(Memcached REQUIRED IMPORTED_TARGET libmemcached)

set(plugin_memcached_SRC
    memcached.cpp
    memcached_p.h
)

set(plugin_memcached_HEADERS
    memcached.h
    Memcached
)

set(target_name Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Memcached)
add_library(${target_name}
    ${plugin_memcached_SRC}
    ${plugin_memcached_HEADERS}
)
add_library(Cutelyst::Memcached ALIAS ${target_name})

generate_export_header(${target_name}
    BASE_NAME CUTELYST_PLUGIN_MEMCACHED
    EXPORT_FILE_NAME ../memcached_export.h
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/../memcached_export.h
    DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins
)

set_target_properties(${target_name} PROPERTIES
    EXPORT_NAME Memcached
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(${target_name}
    PUBLIC Cutelyst::Core
    PRIVATE PkgConfig::Memcached
)

set_property(TARGET ${target_name} PROPERTY PUBLIC_HEADER ${plugin_memcached_HEADERS})
install(TARGETS ${target_name}
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins/Memcached COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQtMemcached.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

if(UNIX)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQtMemcached.5.in
        ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.5
        @ONLY
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.5 DESTINATION ${CMAKE_INSTALL_MANDIR}/man5)
endif()

set(plugin_memcached_TS_FILES
    i18n/plugin_memcached.de.ts
    i18n/plugin_memcached.en.ts
)

qt_add_lupdate(${target_name}
    TS_FILES ${plugin_memcached_TS_FILES}
    OPTIONS -no-obsolete -locations none
)

qt_add_lrelease(${target_name}
    TS_FILES ${plugin_memcached_TS_FILES}
    QM_FILES_OUTPUT_VARIABLE plugin_memcached_QM_FILES
    OPTIONS -idbased
)

install(FILES ${plugin_memcached_QM_FILES} DESTINATION ${I18NDIR})
