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

set(target_name Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Authentication)
add_library(${target_name}
    ${plugin_authentication_SRC}
    ${plugin_authentication_HEADERS}
)
add_library(Cutelyst::Authentication ALIAS ${target_name})

generate_export_header(${target_name} BASE_NAME CUTELYST_PLUGIN_AUTHENTICATION)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/cutelyst_plugin_authentication_export.h
    DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins
)

set_target_properties(${target_name} PROPERTIES
    EXPORT_NAME Authentication
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

if(ENABLE_LTO)
    set_property(TARGET ${target_name} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

target_link_libraries(${target_name}
    PRIVATE Cutelyst::Core
    PRIVATE Cutelyst::Session
)

set_property(TARGET ${target_name} PROPERTY PUBLIC_HEADER ${plugin_authentication_HEADERS})
install(TARGETS ${target_name}
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins/Authentication COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQtAuthentication.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
