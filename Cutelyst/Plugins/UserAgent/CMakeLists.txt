set(plugin_useragent_SRC
    useragent.cpp
)

set(plugin_useragent_HEADERS
    useragent.h
    UserAgent
)

add_library(Cutelyst2Qt5UserAgent
    ${plugin_useragent_SRC}
    ${plugin_useragent_HEADERS}
)
add_library(Cutelyst2Qt5::UserAgent ALIAS Cutelyst2Qt5UserAgent)

set_target_properties(Cutelyst2Qt5UserAgent PROPERTIES
    EXPORT_NAME UserAgent
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst2Qt5UserAgent
    PRIVATE Cutelyst2Qt5::Core
    PRIVATE Qt5::Core
    PRIVATE Qt5::Network
)

set_property(TARGET Cutelyst2Qt5UserAgent PROPERTY PUBLIC_HEADER ${plugin_useragent_HEADERS})
install(TARGETS Cutelyst2Qt5UserAgent
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst2-qt5/Cutelyst/Plugins/ COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5UserAgent.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5UserAgent.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5UserAgent.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
