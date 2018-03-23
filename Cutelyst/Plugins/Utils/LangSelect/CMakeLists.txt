set(plugin_langselect_SRC
    langselect.cpp
    langselect_p.h
)


set(plugin_langselect_HEADERS
    langselect.h
    LangSelect
)

add_library(Cutelyst2Qt5UtilsLangSelect SHARED
    ${plugin_langselect_SRC}
    ${plugin_langselect_HEADERS}
)
add_library(Cutelyst2Qt5::Utils::LangSelect ALIAS Cutelyst2Qt5UtilsLangSelect)

set_target_properties(Cutelyst2Qt5UtilsLangSelect PROPERTIES
    EXPORT_NAME Utils::LangSelect
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst2Qt5UtilsLangSelect
    PUBLIC
        Cutelyst2Qt5::Core
    PRIVATE
        Cutelyst2Qt5::Session
        Qt5::Network
)

set_property(TARGET Cutelyst2Qt5UtilsLangSelect PROPERTY PUBLIC_HEADER ${plugin_langselect_HEADERS})
install(TARGETS Cutelyst2Qt5UtilsLangSelect
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION include/cutelyst2-qt5/Cutelyst/Plugins/Utils
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5UtilsLangSelect.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5UtilsLangSelect.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5UtilsLangSelect.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
