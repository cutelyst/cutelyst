set(plugin_sql_SRC
    sql.cpp
    sql.h
)

set(plugin_sql_HEADERS
    sql.h
    sql.h
    Sql
)

set(target_name Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}UtilsSql)
add_library(${target_name}
    ${plugin_sql_SRC}
    ${plugin_sql_HEADERS}
)
add_library(Cutelyst::Utils::Sql ALIAS ${target_name})

generate_export_header(${target_name}
    BASE_NAME CUTELYST_PLUGIN_UTILS_SQL
    EXPORT_FILE_NAME ../sql_export.h
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/../sql_export.h
    DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins/Utils
)

set_target_properties(${target_name} PROPERTIES
    EXPORT_NAME Utils::Sql
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(${target_name}
    PRIVATE Cutelyst::Core
    PRIVATE Qt::Sql
)

set_property(TARGET ${target_name} PROPERTY PUBLIC_HEADER ${plugin_sql_HEADERS})
install(TARGETS ${target_name}
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins/Utils COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQtUtilsSql.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
