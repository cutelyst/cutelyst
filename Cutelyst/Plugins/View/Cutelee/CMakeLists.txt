include_directories(
    ${Cutelee_INCLUDES}
)

set(cutelee_plugin_SRC
    cuteleeview.cpp
    cuteleeview.h
    cuteleeview_p.h
)

set(plugin_view_cutelee_HEADERS
    cuteleeview.h
)

add_library(Cutelyst2Qt5ViewCutelee ${cutelee_plugin_SRC})

add_library(CutelystQt5::View::Cutelee ALIAS Cutelyst2Qt5ViewCutelee)


set_target_properties(Cutelyst2Qt5ViewCutelee PROPERTIES
    EXPORT_NAME View::Cutelee
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst2Qt5ViewCutelee
    PRIVATE Cutelee5::Templates
    PRIVATE Cutelyst2Qt5::Core
)

set_property(TARGET Cutelyst2Qt5ViewCutelee PROPERTY PUBLIC_HEADER ${plugin_view_cutelee_HEADERS})
install(TARGETS Cutelyst2Qt5ViewCutelee
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst2-qt5/Cutelyst/Plugins/View/Cutelee COMPONENT devel
)

# urifor and csrf_token
if (PLUGIN_CSRFPROTECTION)
add_definitions(-DPLUGIN_CSRFPROTECTION_ENABLED)
endif (PLUGIN_CSRFPROTECTION)

add_library(cutelee_cutelyst MODULE
    urifor.cpp
    urifor.h
    csrf.cpp
    csrf.h
    cutelystcutelee.cpp
    cutelystcutelee.h
)
set_property(TARGET cutelee_cutelyst PROPERTY EXPORT_NAME cutelyst)

target_link_libraries(cutelee_cutelyst
    PRIVATE Cutelee5::Templates
    PRIVATE Cutelyst2Qt5::Core
)
if (PLUGIN_CSRFPROTECTION)
    target_link_libraries(cutelee_cutelyst
        PRIVATE Cutelyst2Qt5::CSRFProtection
    )
endif (PLUGIN_CSRFPROTECTION)
cutelee_adjust_plugin_name(cutelee_cutelyst)

install(TARGETS cutelee_cutelyst EXPORT CutelystTargets
        LIBRARY DESTINATION "${CUTELYST_PLUGINS_DIR}/cutelee/${Cutelee5_VERSION_MAJOR}.${Cutelee5_VERSION_MINOR}"
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5Cutelee.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5Cutelee.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5Cutelee.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
