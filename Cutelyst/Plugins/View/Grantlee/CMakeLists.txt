include_directories(
    ${Grantlee_INCLUDES}
)

set(grantlee_plugin_SRC
    grantleeview.cpp
    grantleeview.h
    grantleeview_p.h
)

set(plugin_view_grantlee_HEADERS
    grantleeview.h
)

add_library(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewGrantlee ${grantlee_plugin_SRC})

add_library(CutelystQt5::View::Grantlee ALIAS Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewGrantlee)


set_target_properties(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewGrantlee PROPERTIES
    EXPORT_NAME View::Grantlee
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewGrantlee
    PRIVATE Grantlee5::Templates
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Core
)

set_property(TARGET Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewGrantlee PROPERTY PUBLIC_HEADER ${plugin_view_grantlee_HEADERS})
install(TARGETS Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ViewGrantlee
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Plugins/View/Grantlee COMPONENT devel
)

# urifor and csrf_token
if (PLUGIN_CSRFPROTECTION)
add_definitions(-DPLUGIN_CSRFPROTECTION_ENABLED)
endif (PLUGIN_CSRFPROTECTION)

add_library(grantlee_cutelyst MODULE
    urifor.cpp
    urifor.h
    csrf.cpp
    csrf.h
    cutelystgrantlee.cpp
    cutelystgrantlee.h
)
set_property(TARGET grantlee_cutelyst PROPERTY EXPORT_NAME cutelyst)

target_link_libraries(grantlee_cutelyst
    PRIVATE Grantlee5::Templates
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Core
)
if (PLUGIN_CSRFPROTECTION)
    target_link_libraries(grantlee_cutelyst
        PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::CSRFProtection
    )
endif (PLUGIN_CSRFPROTECTION)
grantlee_adjust_plugin_name(grantlee_cutelyst)

install(TARGETS grantlee_cutelyst EXPORT CutelystTargets
        LIBRARY DESTINATION "${CUTELYST_PLUGINS_DIR}/grantlee/${Grantlee5_VERSION_MAJOR}.${Grantlee5_VERSION_MINOR}"
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5Grantlee.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Grantlee.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Grantlee.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
