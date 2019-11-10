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

add_library(Cutelyst2Qt5ViewEmail
    ${plugin_view_email_SRC}
    ${plugin_view_email_HEADERS}
)
add_library(CutelystQt5::View::Email ALIAS Cutelyst2Qt5ViewEmail)

set_target_properties(Cutelyst2Qt5ViewEmail PROPERTIES
    EXPORT_NAME View::Email
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst2Qt5ViewEmail
    PRIVATE SimpleMail::Core
    PRIVATE Cutelyst2Qt5::Core
)

set_property(TARGET Cutelyst2Qt5ViewEmail PROPERTY PUBLIC_HEADER ${plugin_view_email_HEADERS})
install(TARGETS Cutelyst2Qt5ViewEmail
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION bin COMPONENT runtime
    ARCHIVE DESTINATION lib COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst2-qt5/Cutelyst/Plugins/View/Email COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5ViewEmail.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5ViewEmail.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5ViewEmail.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
