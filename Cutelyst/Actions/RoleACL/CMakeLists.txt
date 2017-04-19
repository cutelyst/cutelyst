add_library(plugin_action_roleacl MODULE
    roleacl.cpp
    roleacl_p.h
    metadata.json
)

target_link_libraries(plugin_action_roleacl
    PRIVATE cutelyst-qt5
    PRIVATE cutelyst_qt5_plugin_authentication
)

install(TARGETS plugin_action_roleacl DESTINATION ${CUTELYST_PLUGINS_DIR})
