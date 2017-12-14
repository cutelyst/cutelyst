option(USE_JEMALLOC "Use jemalloc memory allocator" OFF)

if (USE_JEMALLOC)
    find_package(JeMalloc REQUIRED)
endif()

set(cutelyst_wsgi_SRC
    wsgi.cpp
    wsgi.h
    wsgi_p.h
    abstractfork.cpp
    abstractfork.h
    protocol.cpp
    protocol.h
    protocolwebsocket.cpp
    protocolwebsocket.h
    protocolhttp.cpp
    protocolhttp.h
    protocolfastcgi.cpp
    protocolfastcgi.h
    postunbuffered.cpp
    postunbuffered.h
    cwsgiengine.cpp
    cwsgiengine.h
    socket.cpp
    socket.h
    tcpserverbalancer.cpp
    tcpserverbalancer.h
    tcpserver.cpp
    tcpserver.h
    tcpsslserver.cpp
    tcpsslserver.h
    localserver.cpp
    localserver.h
    staticmap.cpp
    staticmap.h
)

set(cutelyst_wsgi_HEADERS
    wsgi.h
)

if (UNIX)
    set(cutelyst_wsgi_SRC
        ${cutelyst_wsgi_SRC}
        unixfork.cpp
        unixfork.h
        )
else ()
    set(cutelyst_wsgi_SRC
        ${cutelyst_wsgi_SRC}
        windowsfork.cpp
        windowsfork.h
        )
endif ()

if (LINUX)
    set(cutelyst_wsgi_SRC
        ${cutelyst_wsgi_SRC}
        systemdnotify.cpp
        systemdnotify.h
        )
endif ()

add_library(cutelyst_wsgi_qt5 SHARED ${cutelyst_wsgi_SRC})

add_library(CutelystQt5::WSGI ALIAS cutelyst_wsgi_qt5)
set_property(TARGET cutelyst_wsgi_qt5 PROPERTY EXPORT_NAME WSGI)

set_target_properties(cutelyst_wsgi_qt5 PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(cutelyst_wsgi_qt5
    PRIVATE cutelyst-qt5
)

if (LINUX)
target_link_libraries(cutelyst_wsgi_qt5
    PRIVATE CutelystQt5::EventLoopEPoll
)
endif ()

install(TARGETS cutelyst_wsgi_qt5 EXPORT CutelystQt5Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${cutelyst_wsgi_HEADERS}
        DESTINATION include/cutelyst-qt5/Cutelyst/WSGI
        COMPONENT WSGI
)

set(cutelyst_wsgi_SRCS
    main.cpp
)

add_executable(cutelyst-wsgi
    ${cutelyst_wsgi_SRCS}
)
target_compile_features(cutelyst-wsgi
  PUBLIC
    cxx_generalized_initializers
)
target_link_libraries(cutelyst-wsgi
    cutelyst-qt5
    cutelyst_wsgi_qt5
#    eventdispatcher_epoll
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5WSGI.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/CutelystQt5WSGI.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CutelystQt5WSGI.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

if (JEMALLOC_FOUND)
    target_link_libraries(cutelyst-wsgi ${JEMALLOC_LIBRARIES})
endif()

install(TARGETS cutelyst-wsgi DESTINATION ${INSTALL_BIN_DIR})
