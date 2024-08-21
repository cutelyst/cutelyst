set(target_name ActionRoleACL)
add_library(ActionRoleACL MODULE
    roleacl.cpp
    roleacl.h
    roleacl_p.h
    metadata.json
)

generate_export_header(${target_name} BASE_NAME CUTELYST_PLUGIN_ACTION_ROLEACL)

target_link_libraries(${target_name}
    PRIVATE Cutelyst::Core
    PRIVATE Cutelyst::Authentication
)

if(ENABLE_LTO)
    set_property(TARGET ${target_name} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

install(TARGETS ${target_name} DESTINATION ${PLUGINSDIR})
