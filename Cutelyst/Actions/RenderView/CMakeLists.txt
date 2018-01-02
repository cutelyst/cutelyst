add_library(ActionRenderView MODULE
    renderview.cpp
    renderview.h
    renderview_p.h
    metadata.json
)

target_link_libraries(ActionRenderView
    PRIVATE Cutelyst2Qt5::Core
)

install(TARGETS ActionRenderView DESTINATION ${CUTELYST_PLUGINS_DIR})
