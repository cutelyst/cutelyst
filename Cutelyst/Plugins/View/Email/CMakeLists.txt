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

add_library(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewEmail
    ${plugin_view_email_SRC}
    ${plugin_view_email_HEADERS}
)
add_library(Cutelyst::View::Email ALIAS Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewEmail)

set_target_properties(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewEmail PROPERTIES
    EXPORT_NAME View::Email
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewEmail
    PRIVATE SimpleMail::Core
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Core
)

set_property(TARGET Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewEmail PROPERTY PUBLIC_HEADER ${plugin_view_email_HEADERS})
install(TARGETS Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewEmail
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins/View/Email COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5ViewEmail.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewEmail.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewEmail.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
