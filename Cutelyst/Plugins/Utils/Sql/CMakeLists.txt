set(plugin_sql_SRC
    sql.cpp
    sql.h
)

set(plugin_sql_HEADERS
    sql.h
    sql.h
    Sql
)

add_library(Cutelyst2Qt5UtilsSql
    ${plugin_sql_SRC}
    ${plugin_sql_HEADERS}
)
add_library(Cutelyst2Qt5::Utils::Sql ALIAS Cutelyst2Qt5UtilsSql)

set_target_properties(Cutelyst2Qt5UtilsSql PROPERTIES
    EXPORT_NAME Utils::Sql
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst2Qt5UtilsSql
    PRIVATE Cutelyst2Qt5::Core
    PRIVATE Qt5::Sql
)

set_property(TARGET Cutelyst2Qt5UtilsSql PROPERTY PUBLIC_HEADER ${plugin_sql_HEADERS})
install(TARGETS Cutelyst2Qt5UtilsSql
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION bin COMPONENT runtime
    ARCHIVE DESTINATION lib COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst2-qt5/Cutelyst/Plugins/Utils COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5UtilsSql.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5UtilsSql.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5UtilsSql.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
