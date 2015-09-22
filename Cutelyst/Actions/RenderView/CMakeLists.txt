set(plugin_action_renderview_SRC
    renderview.cpp
    renderview_p.h
    metadata.json
)

add_library(plugin_action_renderview SHARED
    ${plugin_action_renderview_SRC}
)

target_link_libraries(plugin_action_renderview
    cutelyst-qt5
)

install(TARGETS plugin_action_renderview DESTINATION ${CUTELYST_PLUGINS_DIR})
