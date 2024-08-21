set(plugin_view_email_SRC
    viewemail.cpp
    viewemail_p.h
    viewemailtemplate.cpp
    viewemailtemplate_p.h
)

set(plugin_view_email_HEADERS
    viewemail.h
    viewemailtemplate.h
)

set(target_name Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewEmail)
add_library(${target_name}
    ${plugin_view_email_SRC}
    ${plugin_view_email_HEADERS}
)
add_library(Cutelyst::View::Email ALIAS ${target_name})
generate_export_header(${target_name} BASE_NAME CUTELYST_VIEW_EMAIL)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/cutelyst_view_email_export.h
    DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins/View/Email
)

set_target_properties(${target_name} PROPERTIES
    EXPORT_NAME View::Email
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(${target_name}
    PRIVATE SimpleMail::Core
    PRIVATE Cutelyst::Core
)

set_property(TARGET ${target_name} PROPERTY PUBLIC_HEADER ${plugin_view_email_HEADERS})
install(TARGETS ${target_name}
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins/View/Email COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQtViewEmail.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
