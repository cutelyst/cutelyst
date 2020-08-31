option(USE_JEMALLOC "Use jemalloc memory allocator" ${BUILD_ALL})
option(USE_MIMALLOC "Use mimalloc memory allocator" ${BUILD_ALL})
option(USE_MIMALLOC_STATIC "Use mimalloc-static memory allocator" ${BUILD_ALL})
if (USE_JEMALLOC)
    find_package(JeMalloc REQUIRED)
elseif (USE_MIMALLOC OR USE_MIMALLOC_STATIC)
    find_package(mimalloc 1.4 REQUIRED)
    add_definitions(-DUSE_MIMALLOC)
    include_directories(${mimalloc_INCLUDE_DIRECTORIES})
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

add_library(Cutelyst2Qt5Wsgi ${cutelyst_wsgi_SRC})

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
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst2-qt5/Cutelyst/WSGI COMPONENT devel
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
elseif (USE_MIMALLOC)
    target_link_libraries(cutelyst-wsgi2 PUBLIC mimalloc)
elseif (USE_MIMALLOC_STATIC)
    target_link_libraries(cutelyst-wsgi2 PUBLIC mimalloc-static)
endif ()

target_link_libraries(cutelyst-wsgi2
    PRIVATE Qt5::Core
    PRIVATE Cutelyst2Qt5::Core
    PRIVATE Cutelyst2Qt5::WSGI
)
if (LINUX)
target_link_libraries(cutelyst-wsgi2
    PRIVATE Cutelyst2Qt5::EventLoopEPoll
)
endif ()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5WSGI.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5WSGI.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5WSGI.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

install(TARGETS cutelyst-wsgi2
    RUNTIME DESTINATION bin COMPONENT runtime
)
