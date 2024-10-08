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

set(target_name Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewCutelee)
add_library(${target_name} ${cutelee_plugin_SRC})

add_library(Cutelyst::View::Cutelee ALIAS ${target_name})
generate_export_header(${target_name}
    BASE_NAME CUTELYST_VIEW_CUTELEE
    EXPORT_FILE_NAME ../cutelee_export.h
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/../cutelee_export.h
    DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins/View
)

set_target_properties(${target_name} PROPERTIES
    EXPORT_NAME View::Cutelee
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(${target_name}
    PRIVATE
        Cutelee::Templates
        Cutelyst::Core
)
if (PLUGIN_CSRFPROTECTION)
    target_link_libraries(${target_name}
        PRIVATE Cutelyst::CSRFProtection
    )
endif (PLUGIN_CSRFPROTECTION)

set_property(TARGET ${target_name} PROPERTY PUBLIC_HEADER ${plugin_view_cutelee_HEADERS})
install(TARGETS ${target_name}
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins/View/Cutelee COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQtCutelee.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Cutelee.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Cutelee.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

set(plugin_view_cutelee_TS_FILES
    i18n/plugin_view_cutelee.de.ts
    i18n/plugin_view_cutelee.en.ts
)

qt_add_lupdate(${target_name}
    TS_FILES ${plugin_view_cutelee_TS_FILES}
    OPTIONS -no-obsolete -locations none
)

qt_add_lrelease(${target_name}
    TS_FILES ${plugin_view_cutelee_TS_FILES}
    QM_FILES_OUTPUT_VARIABLE plugin_view_cutelee_QM_FILES
    OPTIONS -idbased
)

install(FILES ${plugin_view_cutelee_QM_FILES} DESTINATION ${I18NDIR})
