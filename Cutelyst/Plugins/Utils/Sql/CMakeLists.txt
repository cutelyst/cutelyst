set(plugin_sql_SRC
    sql.cpp
    sql.h
)

set(plugin_sql_HEADERS
    sql.h
    sql.h
    Sql
)

add_library(cutelyst_qt5_plugin_utils_sql SHARED
    ${plugin_sql_SRC}
    ${plugin_sql_HEADERS}
)
add_library(CutelystQt5::Utils::Sql ALIAS cutelyst_qt5_plugin_utils_sql)
set_property(TARGET cutelyst_qt5_plugin_utils_sql PROPERTY EXPORT_NAME Utils::Sql)

set_target_properties(cutelyst_qt5_plugin_utils_sql PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(cutelyst_qt5_plugin_utils_sql
    PRIVATE cutelyst-qt5
    PRIVATE Qt5::Sql
)

install(TARGETS cutelyst_qt5_plugin_utils_sql EXPORT CutelystQt5Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${plugin_sql_HEADERS}
        DESTINATION include/cutelyst-qt5/Cutelyst/Plugins/Utils
        COMPONENT Utils::Sql
)
