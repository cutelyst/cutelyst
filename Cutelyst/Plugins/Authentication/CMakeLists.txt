set(plugin_authentication_SRC
    authenticationuser.cpp
    authenticationrealm.cpp
    authentication.cpp
    authentication_p.h
    authenticationstore.cpp
    minimal.cpp
    htpasswd.cpp
    credentialpassword.cpp
    credentialpassword_p.h
    credentialhttp.cpp
    credentialhttp_p.h
)

set(plugin_authentication_HEADERS
    authenticationstore.h
    authenticationuser.h
    authenticationrealm.h
    authentication.h
    minimal.h
    htpasswd.h
    credentialpassword.h
    credentialhttp.h
)

add_library(cutelyst_qt5_plugin_authentication SHARED
    ${plugin_authentication_SRC}
    ${plugin_authentication_HEADERS}
)
add_library(CutelystQt5::Authentication ALIAS cutelyst_qt5_plugin_authentication)
set_property(TARGET cutelyst_qt5_plugin_authentication PROPERTY EXPORT_NAME Authentication)

set_target_properties(cutelyst_qt5_plugin_authentication PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(cutelyst_qt5_plugin_authentication
    PRIVATE cutelyst-qt5
    PRIVATE cutelyst_qt5_plugin_session
)

install(TARGETS cutelyst_qt5_plugin_authentication EXPORT CutelystQt5Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${plugin_authentication_HEADERS}
        DESTINATION include/cutelyst-qt5/Cutelyst/Plugins/Authentication
        COMPONENT Authentication
)
