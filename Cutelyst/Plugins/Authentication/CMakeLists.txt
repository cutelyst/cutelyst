set(plugin_authentication_SRC
    authenticationuser.cpp
    authenticationrealm.cpp
    authentication.cpp
    authentication_p.h
    authenticationstore.cpp
    minimal.cpp
    htpasswd.cpp
    storeldap.cpp
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
    storeldap.h
    credentialpassword.h
    credentialhttp.h
)

set(target_name Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Authentication)
add_library(${target_name}
    ${plugin_authentication_SRC}
    ${plugin_authentication_HEADERS}
)
add_library(Cutelyst::Authentication ALIAS ${target_name})

generate_export_header(${target_name}
    BASE_NAME CUTELYST_PLUGIN_AUTHENTICATION
    EXPORT_FILE_NAME ../authentication_export.h
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/../authentication_export.h
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

find_package(LDAP QUIET)
if(LDAP_FOUND)
    target_link_libraries(${target_name}
        PRIVATE ${LDAP_LIBRARIES}
    )

    if(DEFINED LDAP_INCLUDE_DIRS)
        target_include_directories(${target_name}
            BEFORE PRIVATE ${LDAP_INCLUDE_DIRS}
        )
    elseif(DEFINED LDAP_INCLUDE_DIR)
        target_include_directories(${target_name}
            BEFORE PRIVATE ${LDAP_INCLUDE_DIR}
        )
    endif()

    target_compile_definitions(${target_name}
        PUBLIC CUTELYST_PLUGIN_AUTHENTICATION_HAS_LDAP
    )
else()
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(LDAP_PKG QUIET IMPORTED_TARGET ldap)
    endif()

    if(LDAP_PKG_FOUND)
        target_link_libraries(${target_name}
            PRIVATE ${LDAP_PKG_LINK_LIBRARIES}
        )
        if(DEFINED LDAP_PKG_INCLUDE_DIRS)
            target_include_directories(${target_name}
                BEFORE PRIVATE ${LDAP_PKG_INCLUDE_DIRS}
            )
        endif()
        if(DEFINED LDAP_PKG_CFLAGS_OTHER)
            target_compile_options(${target_name}
                PRIVATE ${LDAP_PKG_CFLAGS_OTHER}
            )
        endif()
        if(DEFINED LDAP_PKG_LDFLAGS_OTHER)
            target_link_options(${target_name}
                PRIVATE ${LDAP_PKG_LDFLAGS_OTHER}
            )
        endif()
        target_compile_definitions(${target_name}
            PUBLIC CUTELYST_PLUGIN_AUTHENTICATION_HAS_LDAP
        )
        message(STATUS "PLUGIN: Authentication LDAP store, enabled via pkg-config (ldap).")
    else()
        message(STATUS "PLUGIN: Authentication LDAP store, disabled (LDAP dependency not found).")
    endif()
endif()

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
