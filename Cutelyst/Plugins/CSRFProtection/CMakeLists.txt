# SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
# SPDX-License-Identifier: BSD-3-Clause

set(plugin_csrfprotection_SRC
    csrfprotection.cpp
    csrfprotection_p.h
)

set(plugin_csrfprotection_HEADERS
    csrfprotection.h
    CSRFProtection
)

add_library(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}CSRFProtection
    ${plugin_csrfprotection_SRC}
    ${plugin_csrfprotection_HEADERS}
)
add_library(Cutelyst::CSRFProtection ALIAS Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}CSRFProtection)

set_target_properties(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}CSRFProtection PROPERTIES
    EXPORT_NAME CSRFProtection
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}CSRFProtection
    PUBLIC Cutelyst::Core
    PRIVATE Cutelyst::Session
)

set_property(TARGET Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}CSRFProtection PROPERTY PUBLIC_HEADER ${plugin_csrfprotection_HEADERS})
install(TARGETS Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}CSRFProtection
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins/CSRFProtection COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQtCSRFProtection.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}CSRFProtection.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}CSRFProtection.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

if(UNIX)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQtCSRFProtection.5.in
        ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}CSRFProtection.5
        @ONLY
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}CSRFProtection.5 DESTINATION ${CMAKE_INSTALL_MANDIR}/man5)
endif()

set(plugin_csrfprotection_TS_FILES
    i18n/plugin_csrfprotection.de.ts
    i18n/plugin_csrfprotection.en.ts
)

qt_add_lupdate(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}CSRFProtection
    TS_FILES ${plugin_csrfprotection_TS_FILES}
    OPTIONS -no-obsolete -locations none
)

qt_add_lrelease(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}CSRFProtection
    TS_FILES ${plugin_csrfprotection_TS_FILES}
    QM_FILES_OUTPUT_VARIABLE plugin_csrfprotection_QM_FILES
    OPTIONS -idbased
)

install(FILES ${plugin_csrfprotection_QM_FILES} DESTINATION ${I18NDIR})
