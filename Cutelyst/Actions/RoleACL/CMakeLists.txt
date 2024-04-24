add_library(ActionRoleACL MODULE
    roleacl.cpp
    roleacl.h
    roleacl_p.h
    metadata.json
)

target_link_libraries(ActionRoleACL
    PRIVATE Cutelyst::Core
    PRIVATE Cutelyst::Authentication
)

if(ENABLE_LTO)
    set_property(TARGET ActionRoleACL PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

install(TARGETS ActionRoleACL DESTINATION ${PLUGINSDIR})
