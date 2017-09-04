set(plugin_view_json_SRC
    viewjson.cpp
    viewjson_p.h
)

set(plugin_view_json_HEADERS
    viewjson.h
)

add_library(cutelyst_qt5_plugin_view_json SHARED
    ${plugin_view_json_SRC}
    ${plugin_view_json_HEADERS}
)
add_library(CutelystQt5::View::JSON ALIAS cutelyst_qt5_plugin_view_json)
set_property(TARGET cutelyst_qt5_plugin_view_json PROPERTY EXPORT_NAME View::JSON)

set_target_properties(cutelyst_qt5_plugin_view_json PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(cutelyst_qt5_plugin_view_json
    PRIVATE cutelyst-qt5
)

install(TARGETS cutelyst_qt5_plugin_view_json EXPORT CutelystQt5Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${plugin_view_json_HEADERS}
        DESTINATION include/cutelyst-qt5/Cutelyst/Plugins/View/JSON
        COMPONENT View::JSON
)
