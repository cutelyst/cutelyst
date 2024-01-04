add_library(ActionRenderView MODULE
    renderview.cpp
    renderview.h
    renderview_p.h
    metadata.json
)

target_link_libraries(ActionRenderView
    PRIVATE Cutelyst::Core
)

if(ENABLE_LTO)
    set_property(TARGET ActionRenderView PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

install(TARGETS ActionRenderView DESTINATION ${PLUGINSDIR})
