include_directories(
    ${Cutelee_INCLUDES}
)

# urifor and csrf_token
if (PLUGIN_CSRFPROTECTION)
add_definitions(-DPLUGIN_CSRFPROTECTION_ENABLED)
endif (PLUGIN_CSRFPROTECTION)

set(cutelee_plugin_SRC
    urifor.cpp
    urifor.h
    csrf.cpp
    csrf.h
    cutelystcutelee.cpp
    cutelystcutelee.h
    cuteleeview.cpp
    cuteleeview.h
    cuteleeview_p.h
)

set(plugin_view_cutelee_HEADERS
    cuteleeview.h
)

add_library(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewCutelee ${cutelee_plugin_SRC})

add_library(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::View::Cutelee ALIAS Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewCutelee)

set_target_properties(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewCutelee PROPERTIES
    EXPORT_NAME View::Cutelee
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewCutelee
    PRIVATE
        Cutelee::Templates
        Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Core
)
if (PLUGIN_CSRFPROTECTION)
    target_link_libraries(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewCutelee
        PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::CSRFProtection
    )
endif (PLUGIN_CSRFPROTECTION)

set_property(TARGET Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewCutelee PROPERTY PUBLIC_HEADER ${plugin_view_cutelee_HEADERS})
install(TARGETS Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewCutelee
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins/View/Cutelee COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5Cutelee.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Cutelee.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Cutelee.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

set(plugin_view_cutelee_TS_FILES
    i18n/plugin_view_cutelee.de.ts
    i18n/plugin_view_cutelee.en.ts
)

if (${QT_VERSION} VERSION_GREATER "6.2.0")

    qt_add_lupdate(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewCutelee
        TS_FILES ${plugin_view_cutelee_TS_FILES}
        OPTIONS -no-obsolete -locations none
    )

    qt_add_lrelease(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewCutelee
        TS_FILES ${plugin_view_cutelee_TS_FILES}
        QM_FILES_OUTPUT_VARIABLE plugin_view_cutelee_QM_FILES
    )

else ()

    qt_add_translation(plugin_view_cutelee_QM_FILES ${plugin_view_cutelee_TS_FILES})
    add_custom_target(plugin_view_cutelee_TRANSLATIONS ALL
        DEPENDS ${plugin_view_cutelee_QM_FILES}
        SOURCES ${plugin_view_cutelee_TS_FILES}
    )

endif ()

install(FILES ${plugin_view_cutelee_QM_FILES} DESTINATION ${I18NDIR})
