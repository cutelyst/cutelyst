add_library(ActionREST MODULE
    actionrest.cpp
    actionrest.h
    actionrest_p.h
    metadata.json
)

target_link_libraries(ActionREST
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Core
)

if(ENABLE_LTO)
    set_property(TARGET ActionREST PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

install(TARGETS ActionREST DESTINATION ${CUTELYST_PLUGINS_DIR})
