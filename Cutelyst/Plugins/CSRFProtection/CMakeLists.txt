set(plugin_csrfprotection_SRC
    csrfprotection.cpp
    csrfprotection_p.h
)

set(plugin_csrfprotection_HEADERS
    csrfprotection.h
    CSRFProtection
)

add_library(cutelyst_qt5_plugin_csrfprotection SHARED
    ${plugin_csrfprotection_SRC}
    ${plugin_csrfprotection_HEADERS}
)
add_library(CutelystQt5::CSRFProtection ALIAS cutelyst_qt5_plugin_csrfprotection)
set_property(TARGET cutelyst_qt5_plugin_csrfprotection PROPERTY EXPORT_NAME CSRFProtection)

set_target_properties(cutelyst_qt5_plugin_csrfprotection PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(cutelyst_qt5_plugin_csrfprotection
    PRIVATE cutelyst-qt5
    PRIVATE cutelyst_qt5_plugin_session
)

install(TARGETS cutelyst_qt5_plugin_csrfprotection EXPORT CutelystQt5Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${plugin_csrfprotection_HEADERS}
    DESTINATION include/cutelyst-qt5/Cutelyst/Plugins/CSRFProtection
    COMPONENT CSRFProtection
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5CSRFProtection.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/CutelystQt5CSRFProtection.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CutelystQt5CSRFProtection.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
