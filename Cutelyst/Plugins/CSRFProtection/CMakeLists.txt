set(plugin_csrfprotection_SRC
    csrfprotection.cpp
    csrfprotection_p.h
)

set(plugin_csrfprotection_HEADERS
    csrfprotection.h
    CSRFProtection
)

add_library(Cutelyst2Qt5CsrfProtection SHARED
    ${plugin_csrfprotection_SRC}
    ${plugin_csrfprotection_HEADERS}
)
add_library(CutelystQt5::CSRFProtection ALIAS Cutelyst2Qt5CsrfProtection)

set_target_properties(Cutelyst2Qt5CsrfProtection PROPERTIES
    EXPORT_NAME CSRFProtection
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst2Qt5CsrfProtection
    PRIVATE Cutelyst2Qt5::Core
    PRIVATE Cutelyst2Qt5::Session
)

set_property(TARGET Cutelyst2Qt5CsrfProtection PROPERTY PUBLIC_HEADER ${plugin_csrfprotection_HEADERS})
install(TARGETS Cutelyst2Qt5CsrfProtection
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION include/cutelyst2-qt5/Cutelyst/Plugins/CSRFProtection
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5CSRFProtection.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/CutelystQt5CSRFProtection.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CutelystQt5CSRFProtection.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5CSRFProtection.5.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5CSRFProtection.5
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5CSRFProtection.5 DESTINATION ${MANDIR}/man5)
