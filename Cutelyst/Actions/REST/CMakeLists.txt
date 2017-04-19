add_library(plugin_action_rest MODULE
    actionrest.cpp
    actionrest_p.h
    metadata.json
)

target_link_libraries(plugin_action_rest
    PRIVATE cutelyst-qt5
)

install(TARGETS plugin_action_rest DESTINATION ${CUTELYST_PLUGINS_DIR})
