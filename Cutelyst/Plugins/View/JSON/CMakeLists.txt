set(plugin_view_json_SRC
    viewjson.cpp
    viewjson_p.h
)

set(plugin_view_json_HEADERS
    viewjson.h
)

add_library(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewJson
    ${plugin_view_json_SRC}
    ${plugin_view_json_HEADERS}
)
add_library(Cutelyst::View::JSON ALIAS Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewJson)

set_target_properties(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewJson PROPERTIES
    EXPORT_NAME View::JSON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewJson
    PRIVATE Cutelyst::Core
)

set_property(TARGET Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewJson PROPERTY PUBLIC_HEADER ${plugin_view_json_HEADERS})
install(TARGETS Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewJson
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins/View/JSON COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQtViewJSON.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewJSON.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewJSON.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
