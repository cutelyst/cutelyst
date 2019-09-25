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

add_library(Cutelyst2Qt5Authentication
    ${plugin_authentication_SRC}
    ${plugin_authentication_HEADERS}
)
add_library(Cutelyst2Qt5::Authentication ALIAS Cutelyst2Qt5Authentication)

set_target_properties(Cutelyst2Qt5Authentication PROPERTIES
    EXPORT_NAME Authentication
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst2Qt5Authentication
    PRIVATE Cutelyst2Qt5::Core
    PRIVATE Cutelyst2Qt5::Session
)

set_property(TARGET Cutelyst2Qt5Authentication PROPERTY PUBLIC_HEADER ${plugin_authentication_HEADERS})
install(TARGETS Cutelyst2Qt5Authentication
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION bin COMPONENT runtime
    ARCHIVE DESTINATION lib COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst2-qt5/Cutelyst/Plugins/Authentication COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5Authentication.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5Authentication.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5Authentication.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
