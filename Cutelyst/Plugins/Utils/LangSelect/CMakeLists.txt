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

add_library(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}UtilsLangSelect
    ${plugin_langselect_SRC}
    ${plugin_langselect_HEADERS}
)
add_library(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Utils::LangSelect ALIAS Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}UtilsLangSelect)

set_target_properties(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}UtilsLangSelect PROPERTIES
    EXPORT_NAME Utils::LangSelect
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}UtilsLangSelect
    PUBLIC
        Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Core
    PRIVATE
        Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Session
        Qt${QT_VERSION_MAJOR}::Network
)

set_property(TARGET Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}UtilsLangSelect PROPERTY PUBLIC_HEADER ${plugin_langselect_HEADERS})
install(TARGETS Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}UtilsLangSelect
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins/Utils COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5UtilsLangSelect.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}UtilsLangSelect.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}UtilsLangSelect.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
