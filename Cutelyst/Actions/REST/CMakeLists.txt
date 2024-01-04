add_library(ActionREST MODULE
    actionrest.cpp
    actionrest.h
    actionrest_p.h
    metadata.json
)

target_link_libraries(ActionREST
    PRIVATE Cutelyst::Core
)

if(ENABLE_LTO)
    set_property(TARGET ActionREST PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

install(TARGETS ActionREST DESTINATION ${PLUGINSDIR})
