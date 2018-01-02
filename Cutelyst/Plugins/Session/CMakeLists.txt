find_package(PkgConfig QUIET)
if (PkgConfig_FOUND)
    pkg_search_module(MEMCACHED QUIET libmemcached>=1.0.10)
endif (PkgConfig_FOUND)

if (MEMCACHED_FOUND)
    message(STATUS "PLUGIN: Session, enabling SessionStoreMemcached.")
    set(sessionstorememcached_SRC sessionstorememcached.cpp sessionstorememcached_p.h)
    set(sessionstorememcached_HEADERS sessionstorememcached.h)
endif (MEMCACHED_FOUND)

set(plugin_session_SRC
    ${sessionstorememcached_SRC}
    sessionstorefile.cpp
    session.cpp
    session_p.h
)

set(plugin_session_HEADERS
    ${sessionstorememcached_HEADERS}
    sessionstorefile.h
    session.h
    Session
)

add_library(Cutelyst2Qt5Session SHARED
    ${plugin_session_SRC}
    ${plugin_session_HEADERS}
)
add_library(Cutelyst2Qt5::Session ALIAS Cutelyst2Qt5Session)

set_target_properties(Cutelyst2Qt5Session PROPERTIES
    EXPORT_NAME Session
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

if (MEMCACHED_FOUND)
    target_include_directories(Cutelyst2Qt5Session
        PUBLIC ${MEMCACHED_INCLUDE_DIRS}
    )
endif (MEMCACHED_FOUND)

target_link_libraries(Cutelyst2Qt5Session
    PRIVATE Cutelyst2Qt5::Core
    ${MEMCACHED_LIBRARIES}
)

set_property(TARGET Cutelyst2Qt5Session PROPERTY PUBLIC_HEADER ${plugin_session_HEADERS})
install(TARGETS Cutelyst2Qt5Session
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION include/cutelyst2-qt5/Cutelyst/Plugins/Session
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5Session.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5Session.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5Session.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
