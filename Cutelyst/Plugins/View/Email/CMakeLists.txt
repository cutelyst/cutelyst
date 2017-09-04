include_directories(
    ${SimpleMailQt5_INCLUDE_DIR}
)

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

add_library(cutelyst_qt5_plugin_view_email SHARED
    ${plugin_view_email_SRC}
    ${plugin_view_email_HEADERS}
)
add_library(CutelystQt5::View::Email ALIAS cutelyst_qt5_plugin_view_email)
set_property(TARGET cutelyst_qt5_plugin_view_email PROPERTY EXPORT_NAME View::Email)

set_target_properties(cutelyst_qt5_plugin_view_email PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(cutelyst_qt5_plugin_view_email
    PRIVATE SimpleMail::Core
    PRIVATE cutelyst-qt5
)

install(TARGETS cutelyst_qt5_plugin_view_email EXPORT CutelystQt5Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${plugin_view_email_HEADERS}
        DESTINATION include/cutelyst-qt5/Cutelyst/Plugins/View/Email
        COMPONENT View::Email
)
