set(plugin_session_SRC
    sessionstorefile.cpp
    session.cpp
    session_p.h
)

set(plugin_session_HEADERS
    sessionstorefile.h
    session.h
    Session
)

add_library(cutelyst_qt5_plugin_session SHARED
    ${plugin_session_SRC}
    ${plugin_session_HEADERS}
)
add_library(CutelystQt5::Session ALIAS cutelyst_qt5_plugin_session)
set_property(TARGET cutelyst_qt5_plugin_session PROPERTY EXPORT_NAME Session)

set_target_properties(cutelyst_qt5_plugin_session PROPERTIES
    VERSION ${CUTELYST_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(cutelyst_qt5_plugin_session
    PRIVATE cutelyst-qt5
)

install(TARGETS cutelyst_qt5_plugin_session EXPORT CutelystQt5Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${plugin_session_HEADERS}
        DESTINATION include/cutelyst-qt5/Cutelyst/Plugins/Session
        COMPONENT Session
)
