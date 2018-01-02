add_library(ActionREST MODULE
    actionrest.cpp
    actionrest.h
    actionrest_p.h
    metadata.json
)

target_link_libraries(ActionREST
    PRIVATE Cutelyst2Qt5::Core
)

install(TARGETS ActionREST DESTINATION ${CUTELYST_PLUGINS_DIR})
