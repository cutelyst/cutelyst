set(plugin_pagination_SRC
    pagination.cpp
    pagination.h
)

set(plugin_pagination_HEADERS
    pagination.h
    Pagination
)

set(target_name Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}UtilsPagination)
add_library(${target_name}
    ${plugin_pagination_SRC}
    ${plugin_pagination_HEADERS}
)
add_library(Cutelyst::Utils::Pagination ALIAS ${target_name})

generate_export_header(${target_name}
    BASE_NAME CUTELYST_PLUGIN_UTILS_PAGINATION
    EXPORT_FILE_NAME ../pagination_export.h
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/../pagination_export.h
    DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins/Utils
)

set_target_properties(${target_name} PROPERTIES
    EXPORT_NAME Utils::Pagination
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(${target_name}
    PRIVATE Cutelyst::Core
)

set_property(TARGET ${target_name} PROPERTY PUBLIC_HEADER ${plugin_pagination_HEADERS})
install(TARGETS ${target_name}
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins/Utils COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQtUtilsPagination.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
