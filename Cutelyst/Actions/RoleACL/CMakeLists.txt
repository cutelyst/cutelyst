set(target_name CutelystRoleACL)
qt_add_plugin(${target_name}
    ${CUTELYST_PLUGIN_LINK_TYPE}
)

target_sources(${target_name} PRIVATE
    roleacl.cpp
    roleacl.h
    roleacl_p.h
    metadata.json
)

target_link_libraries(${target_name}
    PRIVATE Cutelyst::Core
    PRIVATE Cutelyst::Authentication
)

if(ENABLE_LTO)
    set_property(TARGET ${target_name} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

install(TARGETS ${target_name} DESTINATION ${PLUGINSDIR})
