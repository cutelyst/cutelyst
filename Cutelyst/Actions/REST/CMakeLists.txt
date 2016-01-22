set(plugin_action_rest_SRC
    actionrest.cpp
    actionrest_p.h
    metadata.json
)

add_library(plugin_action_rest SHARED
    ${plugin_action_rest_SRC}
)

target_link_libraries(plugin_action_rest
    PRIVATE cutelyst-qt5
)

install(TARGETS plugin_action_rest DESTINATION ${CUTELYST_PLUGINS_DIR})
