add_library(plugin_action_renderview MODULE
    renderview.cpp
    renderview_p.h
    metadata.json
)

target_link_libraries(plugin_action_renderview
    PRIVATE cutelyst-qt5
)

install(TARGETS plugin_action_renderview DESTINATION ${CUTELYST_PLUGINS_DIR})
