include_directories(
    ${Grantlee_INCLUDES}
)

add_definitions(-DQT_PLUGIN)

set(grantlee_plugin_SRC
    grantleeview.cpp
    grantleeview_p.h
)

set(plugin_view_grantlee_HEADERS
    grantleeview.h
)

add_library(cutelyst_view_grantlee SHARED ${grantlee_plugin_SRC})

add_library(CutelystQt5::View::Grantlee ALIAS cutelyst_view_grantlee)

set_property(TARGET cutelyst_view_grantlee PROPERTY EXPORT_NAME View::Grantlee)

set_target_properties(cutelyst_view_grantlee PROPERTIES
    VERSION ${CUTELYST_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(cutelyst_view_grantlee
    LINK_PRIVATE Grantlee5::Templates
    cutelyst-qt5
)

install(TARGETS cutelyst_view_grantlee EXPORT CutelystQt5Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${plugin_view_grantlee_HEADERS}
        DESTINATION include/cutelyst-qt5/Cutelyst/Plugins/View/Grantlee
        COMPONENT View::Grantlee
)
