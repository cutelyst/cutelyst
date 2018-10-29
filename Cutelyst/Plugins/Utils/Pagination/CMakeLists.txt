set(plugin_pagination_SRC
    pagination.cpp
    pagination.h
)

set(plugin_pagination_HEADERS
    pagination.h
    Pagination
)

add_library(Cutelyst2Qt5UtilsPagination SHARED
    ${plugin_pagination_SRC}
    ${plugin_pagination_HEADERS}
)
add_library(Cutelyst2Qt5::Utils::Pagination ALIAS Cutelyst2Qt5UtilsPagination)

set_target_properties(Cutelyst2Qt5UtilsPagination PROPERTIES
    EXPORT_NAME Utils::Pagination
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst2Qt5UtilsPagination
    PRIVATE Cutelyst2Qt5::Core
)

set_property(TARGET Cutelyst2Qt5UtilsPagination PROPERTY PUBLIC_HEADER ${plugin_pagination_HEADERS})
install(TARGETS Cutelyst2Qt5UtilsPagination
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION bin COMPONENT runtime
    ARCHIVE DESTINATION lib COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst2-qt5/Cutelyst/Plugins/Utils COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5UtilsPagination.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5UtilsPagination.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5UtilsPagination.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
