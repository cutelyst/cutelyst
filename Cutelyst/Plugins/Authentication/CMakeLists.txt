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

add_library(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Authentication
    ${plugin_authentication_SRC}
    ${plugin_authentication_HEADERS}
)
add_library(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Authentication ALIAS Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Authentication)

set_target_properties(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Authentication PROPERTIES
    EXPORT_NAME Authentication
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

if(ENABLE_LTO)
    set_property(TARGET Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Authentication PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

target_link_libraries(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Authentication
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Core
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Session
)

set_property(TARGET Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Authentication PROPERTY PUBLIC_HEADER ${plugin_authentication_HEADERS})
install(TARGETS Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Authentication
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins/Authentication COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5Authentication.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Authentication.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Authentication.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
