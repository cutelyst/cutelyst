add_library(ActionRenderView MODULE
    renderview.cpp
    renderview.h
    renderview_p.h
    metadata.json
)

target_link_libraries(ActionRenderView
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Core
)

install(TARGETS ActionRenderView DESTINATION ${CUTELYST_PLUGINS_DIR})
