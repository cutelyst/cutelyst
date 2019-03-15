include_directories(
    /usr/include/ClearSilver
)

set(clearsilver_plugin_SRC
    clearsilver.cpp
    clearsilver.h
    clearsilver_p.h
)

set(plugin_view_clearsilver_HEADERS
    clearsilver.h
)

add_library(Cutelyst2Qt5ViewClearSilver
    ${clearsilver_plugin_SRC}
)
add_library(Cutelyst2Qt5::View::ClearSilver ALIAS Cutelyst2Qt5ViewClearSilver)

set_target_properties(Cutelyst2Qt5ViewClearSilver PROPERTIES
    EXPORT_NAME View::ClearSilver
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst2Qt5ViewClearSilver
    PRIVATE Cutelyst2Qt5::Core
    PRIVATE ${ClearSilver_CS}
    PRIVATE ${ClearSilver_Util}
)

set_property(TARGET Cutelyst2Qt5ViewClearSilver PROPERTY PUBLIC_HEADER ${plugin_view_clearsilver_HEADERS})
install(TARGETS Cutelyst2Qt5ViewClearSilver
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION bin COMPONENT runtime
    ARCHIVE DESTINATION lib COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst2-qt5/Cutelyst/Plugins/View/ClearSilver COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5ViewClearSilver.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5ViewClearSilver.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5ViewClearSilver.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
