option(USE_JEMALLOC "Use jemalloc memory allocator" ${BUILD_ALL})
if (USE_JEMALLOC)
    find_package(JeMalloc REQUIRED)
endif ()

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
    hpacktables.cpp
    hpacktables.h
    protocolhttp2.cpp
    protocolhttp2.h
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

add_library(Cutelyst2Qt5Wsgi SHARED ${cutelyst_wsgi_SRC})

add_library(Cutelyst2Qt5::WSGI ALIAS Cutelyst2Qt5Wsgi)

set_target_properties(Cutelyst2Qt5Wsgi PROPERTIES
    EXPORT_NAME WSGI
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst2Qt5Wsgi
    PRIVATE Cutelyst2Qt5::Core
)

if (LINUX)
target_link_libraries(Cutelyst2Qt5Wsgi
    PRIVATE Cutelyst2Qt5::EventLoopEPoll
)
endif ()

set_property(TARGET Cutelyst2Qt5Wsgi PROPERTY PUBLIC_HEADER ${cutelyst_wsgi_HEADERS})
install(TARGETS Cutelyst2Qt5Wsgi
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION include/cutelyst2-qt5/Cutelyst/WSGI
)

set(cutelyst_wsgi_SRCS
    main.cpp
)

add_executable(cutelyst-wsgi2
    ${cutelyst_wsgi_SRCS}
)

target_compile_features(cutelyst-wsgi2
  PUBLIC
    cxx_generalized_initializers
)
if (JEMALLOC_FOUND)
    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " -Wl,--no-as-needed")
    target_link_libraries(cutelyst-wsgi2 PRIVATE ${JEMALLOC_LIBRARIES})
endif()

target_link_libraries(cutelyst-wsgi2
    PRIVATE Qt5::Core
    PRIVATE Cutelyst2Qt5::WSGI
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5WSGI.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5WSGI.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5WSGI.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

install(TARGETS cutelyst-wsgi2 DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)
