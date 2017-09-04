set(plugin_staticsimple_SRC
    staticsimple.cpp
    staticsimple_p.h
    staticsimple.h
)

set(plugin_staticsimple_HEADERS
    staticsimple.h
    StaticSimple
)

add_library(cutelyst_qt5_plugin_staticsimple SHARED
    ${plugin_staticsimple_SRC}
    ${plugin_staticsimple_HEADERS}
)
add_library(CutelystQt5::StaticSimple ALIAS cutelyst_qt5_plugin_staticsimple)
set_property(TARGET cutelyst_qt5_plugin_staticsimple PROPERTY EXPORT_NAME StaticSimple)

set_target_properties(cutelyst_qt5_plugin_staticsimple PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(cutelyst_qt5_plugin_staticsimple
    PRIVATE cutelyst-qt5
)

install(TARGETS cutelyst_qt5_plugin_staticsimple EXPORT CutelystQt5Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${plugin_staticsimple_HEADERS}
        DESTINATION include/cutelyst-qt5/Cutelyst/Plugins/StaticSimple
        COMPONENT StaticSimple
)
