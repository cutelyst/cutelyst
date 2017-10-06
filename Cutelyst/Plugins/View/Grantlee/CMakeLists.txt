include_directories(
    ${Grantlee_INCLUDES}
)

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
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(cutelyst_view_grantlee
    PRIVATE Grantlee5::Templates
    PRIVATE cutelyst-qt5
)

install(TARGETS cutelyst_view_grantlee EXPORT CutelystQt5Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${plugin_view_grantlee_HEADERS}
        DESTINATION include/cutelyst-qt5/Cutelyst/Plugins/View/Grantlee
        COMPONENT View::Grantlee
)

# urifor
add_library(grantlee_cutelyst MODULE
    urifor.cpp
    urifor.h
    cutelystgrantlee.cpp
)
set_property(TARGET grantlee_cutelyst PROPERTY EXPORT_NAME cutelyst)

target_link_libraries(grantlee_cutelyst
    PRIVATE Grantlee5::Templates
    PRIVATE cutelyst-qt5
)
grantlee_adjust_plugin_name(grantlee_cutelyst)

install(TARGETS grantlee_cutelyst EXPORT CutelystQt5Targets
        LIBRARY DESTINATION "${CUTELYST_PLUGINS_DIR}/grantlee/${Grantlee5_VERSION_MAJOR}.${Grantlee5_VERSION_MINOR}"
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5Grantlee.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/CutelystQt5Grantlee.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CutelystQt5Grantlee.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
