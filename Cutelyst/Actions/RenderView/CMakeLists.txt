set(target_name ActionRenderView)
add_library(${target_name} MODULE
    renderview.cpp
    renderview.h
    renderview_p.h
    metadata.json
)
generate_export_header(${target_name} BASE_NAME CUTELYST_ACTION_RENDERVIEW)

target_link_libraries(${target_name}
    PRIVATE Cutelyst::Core
)

if(ENABLE_LTO)
    set_property(TARGET ${target_name} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

install(TARGETS ${target_name} DESTINATION ${PLUGINSDIR})
