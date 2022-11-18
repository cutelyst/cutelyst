# SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
# SPDX-License-Identifier: BSD-3-Clause

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

add_library(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Memcached
    ${plugin_memcached_SRC}
    ${plugin_memcached_HEADERS}
)
add_library(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Memcached ALIAS Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Memcached)

set_target_properties(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Memcached PROPERTIES
    EXPORT_NAME Memcached
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Memcached
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Core
    ${MEMCACHED_LIBRARIES}
)

set_property(TARGET Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Memcached PROPERTY PUBLIC_HEADER ${plugin_memcached_HEADERS})
install(TARGETS Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Memcached
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins/Memcached COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5Memcached.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Memcached.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Memcached.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

if(UNIX)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5Memcached.5.in
        ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Memcached.5
        @ONLY
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Memcached.5 DESTINATION ${MANDIR}/man5)
endif()

set(plugin_memcached_TS_FILES
    i18n/plugin_memcached.de.ts
    i18n/plugin_memcached.en.ts
)

if (${QT_VERSION} VERSION_GREATER "6.2.0")

    qt_add_lupdate(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Memcached
        TS_FILES ${plugin_memcached_TS_FILES}
        OPTIONS -no-obsolete -locations none
    )

    qt_add_lrelease(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Memcached
        TS_FILES ${plugin_memcached_TS_FILES}
        QM_FILES_OUTPUT_VARIABLE plugin_memcached_QM_FILES
    )

else ()

    qt_add_translation(plugin_memcached_QM_FILES ${plugin_memcached_TS_FILES})
    add_custom_target(plugin_memcached_TRANSLATIONS ALL
        DEPENDS ${plugin_memcached_QM_FILES}
        SOURCES ${plugin_memcached_TS_FILES}
    )

endif ()

install(FILES ${plugin_memcached_QM_FILES} DESTINATION ${I18NDIR})
