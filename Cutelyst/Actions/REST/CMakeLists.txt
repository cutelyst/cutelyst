set(target_name ActionREST)
add_library(ActionREST MODULE
    actionrest.cpp
    actionrest.h
    actionrest_p.h
    metadata.json
)

generate_export_header(${target_name} BASE_NAME CUTELYST_PLUGIN_ACTION_REST)

target_link_libraries(${target_name}
    PRIVATE Cutelyst::Core
)

if(ENABLE_LTO)
    set_property(TARGET ${target_name} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

install(TARGETS ${target_name} DESTINATION ${PLUGINSDIR})
