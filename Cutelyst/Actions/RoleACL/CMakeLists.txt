add_library(ActionRoleACL MODULE
    roleacl.cpp
    roleacl.h
    roleacl_p.h
    metadata.json
)

target_link_libraries(ActionRoleACL
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Core
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Authentication
)

if(ENABLE_LTO)
    set_property(TARGET ActionRoleACL PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

install(TARGETS ActionRoleACL DESTINATION ${CUTELYST_PLUGINS_DIR})
