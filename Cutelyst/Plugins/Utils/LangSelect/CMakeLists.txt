# SPDX-FileCopyrightText: (C) 2018-2022 Matthias Fehring <mf@huessenbergnetz.de>
# SPDX-License-Identifier: BSD-3-Clause

set(plugin_langselect_SRC
    langselect.cpp
    langselect_p.h
)


set(plugin_langselect_HEADERS
    langselect.h
    LangSelect
)

set(target_name Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}UtilsLangSelect)
add_library(${target_name}
    ${plugin_langselect_SRC}
    ${plugin_langselect_HEADERS}
)
add_library(Cutelyst::Utils::LangSelect ALIAS ${target_name})

generate_export_header(${target_name} BASE_NAME CUTELYST_PLUGIN_UTILS_LANGSELECT)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/cutelyst_plugin_utils_langselect_export.h
    DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins/Utils
)

set_target_properties(${target_name} PROPERTIES
    EXPORT_NAME Utils::LangSelect
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(${target_name}
    PUBLIC
        Cutelyst::Core
    PRIVATE
        Cutelyst::Session
        Qt::Network
)

set_property(TARGET ${target_name} PROPERTY PUBLIC_HEADER ${plugin_langselect_HEADERS})
install(TARGETS ${target_name}
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins/Utils COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQtUtilsLangSelect.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

if(UNIX)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQtLangSelect.5.in
        ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}LangSelect.5
        @ONLY
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}LangSelect.5 DESTINATION ${CMAKE_INSTALL_MANDIR}/man5)
endif()
