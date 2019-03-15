set(plugin_statusmessage_SRC
    statusmessage.cpp
    statusmessage.h
)

set(plugin_statusmessage_HEADERS
    statusmessage.h
    StatusMessage
)

add_library(Cutelyst2Qt5StatusMessage
    ${plugin_statusmessage_SRC}
    ${plugin_statusmessage_HEADERS}
)
add_library(Cutelyst2Qt5::StatusMessage ALIAS Cutelyst2Qt5StatusMessage)

set_target_properties(Cutelyst2Qt5StatusMessage PROPERTIES
    EXPORT_NAME StatusMessage
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst2Qt5StatusMessage
    PRIVATE Cutelyst2Qt5::Core
    PRIVATE Cutelyst2Qt5::Session
)

set_property(TARGET Cutelyst2Qt5StatusMessage PROPERTY PUBLIC_HEADER ${plugin_statusmessage_HEADERS})
install(TARGETS Cutelyst2Qt5StatusMessage
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION bin COMPONENT runtime
    ARCHIVE DESTINATION lib COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst2-qt5/Cutelyst/Plugins COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5StatusMessage.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5StatusMessage.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5StatusMessage.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
