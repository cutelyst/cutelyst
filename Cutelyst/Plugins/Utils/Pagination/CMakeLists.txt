set(plugin_pagination_SRC
    pagination.cpp
    pagination.h
)

set(plugin_pagination_HEADERS
    pagination.h
    Pagination
)

add_library(cutelyst_qt5_plugin_utils_pagination SHARED
    ${plugin_pagination_SRC}
    ${plugin_pagination_HEADERS}
)
add_library(CutelystQt5::Utils::Pagination ALIAS cutelyst_qt5_plugin_utils_pagination)
set_property(TARGET cutelyst_qt5_plugin_utils_pagination PROPERTY EXPORT_NAME Utils::Pagination)

set_target_properties(cutelyst_qt5_plugin_utils_pagination PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(cutelyst_qt5_plugin_utils_pagination
    PRIVATE cutelyst-qt5
)

install(TARGETS cutelyst_qt5_plugin_utils_pagination EXPORT CutelystQt5Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${plugin_pagination_HEADERS}
        DESTINATION include/cutelyst-qt5/Cutelyst/Plugins/Utils
        COMPONENT Utils::Pagination
)
