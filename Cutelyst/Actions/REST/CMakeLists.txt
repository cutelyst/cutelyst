set(target_name CutelystActionREST)
qt_add_plugin(${target_name}
    ${CUTELYST_PLUGIN_LINK_TYPE}
    OUTPUT_TARGETS actionrest_out
)

target_sources(${target_name} PRIVATE
    actionrest.cpp
    actionrest.h
    actionrest_p.h
    metadata.json
)

target_link_libraries(${target_name}
    PRIVATE Cutelyst::Core
)

if(ENABLE_LTO)
    set_property(TARGET ${target_name} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

install(TARGETS ${actionrest_out} DESTINATION ${PLUGINSDIR})
