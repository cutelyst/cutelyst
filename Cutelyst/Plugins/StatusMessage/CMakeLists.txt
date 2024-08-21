set(plugin_statusmessage_SRC
    statusmessage.cpp
    statusmessage.h
)

set(plugin_statusmessage_HEADERS
    statusmessage.h
    StatusMessage
)

set(target_name Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}StatusMessage)
add_library(${target_name}
    ${plugin_statusmessage_SRC}
    ${plugin_statusmessage_HEADERS}
)
add_library(Cutelyst::StatusMessage ALIAS ${target_name})

generate_export_header(${target_name} BASE_NAME CUTELYST_PLUGIN_STATUSMESSAGE)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/cutelyst_plugin_statusmessage_export.h
    DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins
)

set_target_properties(${target_name} PROPERTIES
    EXPORT_NAME StatusMessage
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(${target_name}
    PRIVATE Cutelyst::Core
    PRIVATE Cutelyst::Session
)

set_property(TARGET ${target_name} PROPERTY PUBLIC_HEADER ${plugin_statusmessage_HEADERS})
install(TARGETS ${target_name}
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQtStatusMessage.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
