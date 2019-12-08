set(plugin_staticsimple_SRC
    staticsimple.cpp
    staticsimple_p.h
    staticsimple.h
)

set(plugin_staticsimple_HEADERS
    staticsimple.h
    StaticSimple
)

add_library(Cutelyst2Qt5StaticSimple
    ${plugin_staticsimple_SRC}
    ${plugin_staticsimple_HEADERS}
)
add_library(Cutelyst2Qt5::StaticSimple ALIAS Cutelyst2Qt5StaticSimple)

set_target_properties(Cutelyst2Qt5StaticSimple PROPERTIES
    EXPORT_NAME StaticSimple
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst2Qt5StaticSimple
    PRIVATE Cutelyst2Qt5::Core
)

set_property(TARGET Cutelyst2Qt5StaticSimple PROPERTY PUBLIC_HEADER ${plugin_staticsimple_HEADERS})
install(TARGETS Cutelyst2Qt5StaticSimple
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst2-qt5/Cutelyst/Plugins/StaticSimple COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5StaticSimple.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5StaticSimple.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5StaticSimple.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
