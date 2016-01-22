set(plugin_action_roleacl_SRC
    roleacl.cpp
    roleacl_p.h
    metadata.json
)

add_library(plugin_action_roleacl SHARED
    ${plugin_action_roleacl_SRC}
)

target_link_libraries(plugin_action_roleacl
    PRIVATE cutelyst-qt5
    PRIVATE cutelyst_qt5_plugin_authentication
)

install(TARGETS plugin_action_roleacl DESTINATION ${CUTELYST_PLUGINS_DIR})
