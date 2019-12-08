set(plugin_view_json_SRC
    viewjson.cpp
    viewjson_p.h
)

set(plugin_view_json_HEADERS
    viewjson.h
)

add_library(Cutelyst2Qt5ViewJson
    ${plugin_view_json_SRC}
    ${plugin_view_json_HEADERS}
)
add_library(Cutelyst2Qt5::View::JSON ALIAS Cutelyst2Qt5ViewJson)

set_target_properties(Cutelyst2Qt5ViewJson PROPERTIES
    EXPORT_NAME View::JSON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst2Qt5ViewJson
    PRIVATE Cutelyst2Qt5::Core
)

set_property(TARGET Cutelyst2Qt5ViewJson PROPERTY PUBLIC_HEADER ${plugin_view_json_HEADERS})
install(TARGETS Cutelyst2Qt5ViewJson
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst2-qt5/Cutelyst/Plugins/View/JSON COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5ViewJSON.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5ViewJSON.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5ViewJSON.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
