include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(plugin_action_rest_SRC
    actionrest.cpp
    actionrest_p.h
)

set(plugin_action_rest_HEADERS
    actionrest.h
)

add_definitions(
    -std=c++11
)

add_library(cutelyst_qt5_plugin_action_rest SHARED
    ${plugin_action_rest_SRC}
    ${plugin_action_rest_HEADERS}
)
add_library(CutelystQt5::Action::REST ALIAS cutelyst_qt5_plugin_action_rest)
set_property(TARGET cutelyst_qt5_plugin_action_rest PROPERTY EXPORT_NAME Action::REST)

set_target_properties(cutelyst_qt5_plugin_action_rest PROPERTIES
    VERSION ${CUTELYST_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(cutelyst_qt5_plugin_action_rest
    cutelyst-qt5
    Qt5::Core
    Qt5::Network
)

install(TARGETS cutelyst_qt5_plugin_action_rest EXPORT CutelystQt5Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${plugin_action_rest_HEADERS}
        DESTINATION include/cutelyst-qt5/Cutelyst/Plugins
        COMPONENT Action::REST
)