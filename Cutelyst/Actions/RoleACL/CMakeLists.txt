include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(plugin_action_roleacl_SRC
    roleacl.cpp
    roleacl_p.h
    metadata.json
)

add_definitions(
    -std=c++11
)

add_library(plugin_action_roleacl SHARED
    ${plugin_action_roleacl_SRC}
)

target_link_libraries(plugin_action_roleacl
    cutelyst-qt5
    cutelyst_qt5_plugin_authentication
)

install(TARGETS plugin_action_roleacl DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cutelyst-plugins)
