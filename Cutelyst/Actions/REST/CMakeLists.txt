include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(plugin_action_rest_SRC
    actionrest.cpp
    actionrest_p.h
    metadata.json
)

add_definitions(
    -std=c++11
)

add_library(plugin_action_rest SHARED
    ${plugin_action_rest_SRC}
)

target_link_libraries(plugin_action_rest
    cutelyst-qt5
)

install(TARGETS plugin_action_rest DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cutelyst-plugins)
