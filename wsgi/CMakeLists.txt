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
    hpack_p.cpp
    hpack_p.h
    hpack.cpp
    hpack.h
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

add_library(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Wsgi ${cutelyst_wsgi_SRC})

add_library(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::WSGI ALIAS Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Wsgi)

set_target_properties(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Wsgi PROPERTIES
    EXPORT_NAME WSGI
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Wsgi
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Core
)

if (LINUX)
target_link_libraries(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Wsgi
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::EventLoopEPoll
)
endif ()

set_property(TARGET Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Wsgi PROPERTY PUBLIC_HEADER ${cutelyst_wsgi_HEADERS})
install(TARGETS Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Wsgi
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/WSGI COMPONENT devel
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
if (JEMALLOC_FOUND)
    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " -Wl,--no-as-needed")
    target_link_libraries(cutelyst-wsgi PRIVATE ${JEMALLOC_LIBRARIES})
endif()

target_link_libraries(cutelyst-wsgi
    PRIVATE Qt${QT_VERSION_MAJOR}::Core
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Core
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::WSGI
)
if (LINUX)
target_link_libraries(cutelyst-wsgi
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::EventLoopEPoll
)
endif ()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5WSGI.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}WSGI.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}WSGI.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

set_target_properties(cutelyst-wsgi PROPERTIES OUTPUT_NAME cutelyst${PROJECT_VERSION_MAJOR}-wsgi${QT_VERSION_MAJOR})
install(TARGETS cutelyst-wsgi
    RUNTIME DESTINATION bin COMPONENT runtime
)
