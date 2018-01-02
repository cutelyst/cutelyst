add_library(ActionRoleACL MODULE
    roleacl.cpp
    roleacl.h
    roleacl_p.h
    metadata.json
)

target_link_libraries(ActionRoleACL
    PRIVATE Cutelyst2Qt5::Core
    PRIVATE Cutelyst2Qt5::Authentication
)

install(TARGETS ActionRoleACL DESTINATION ${CUTELYST_PLUGINS_DIR})
