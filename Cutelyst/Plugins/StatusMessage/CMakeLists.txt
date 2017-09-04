set(plugin_statusmessage_SRC
    statusmessage.cpp
    statusmessage.h
)

set(plugin_statusmessage_HEADERS
    statusmessage.h
    StatusMessage
)

add_library(cutelyst_qt5_plugin_statusmessage SHARED
    ${plugin_statusmessage_SRC}
    ${plugin_statusmessage_HEADERS}
)
add_library(CutelystQt5::StatusMessage ALIAS cutelyst_qt5_plugin_statusmessage)
set_property(TARGET cutelyst_qt5_plugin_statusmessage PROPERTY EXPORT_NAME StatusMessage)

set_target_properties(cutelyst_qt5_plugin_statusmessage PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(cutelyst_qt5_plugin_statusmessage
    PRIVATE cutelyst-qt5
    PRIVATE cutelyst_qt5_plugin_session
)

install(TARGETS cutelyst_qt5_plugin_statusmessage EXPORT CutelystQt5Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${plugin_statusmessage_HEADERS}
    DESTINATION include/cutelyst-qt5/Cutelyst/Plugins
    COMPONENT StatusMessage
)
