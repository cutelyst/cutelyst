option(USE_JEMALLOC "Use jemalloc memory allocator" ${BUILD_ALL})
if (USE_JEMALLOC)
    find_package(JeMalloc REQUIRED)
endif ()

set(cutelyst_server_SRC
    server.cpp
    server.h
    server_p.h
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
    serverengine.cpp
    serverengine.h
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

set(cutelyst_server_HEADERS
    server.h
)

if (UNIX)
    list(APPEND cutelyst_server_SRC
        unixfork.cpp
        unixfork.h
        )
else ()
    list(APPEND cutelyst_server_SRC
        windowsfork.cpp
        windowsfork.h
        )
endif ()

if (LINUX)
    list(APPEND cutelyst_server_SRC
        systemdnotify.cpp
        systemdnotify.h
        )
endif ()

set(target_name Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Server)

add_library(${target_name} ${cutelyst_server_SRC})

add_library(Cutelyst::Server ALIAS ${target_name})

generate_export_header(${target_name} BASE_NAME CUTELYST_SERVER)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/cutelyst_server_export.h
    DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Server
)

set_target_properties(${target_name} PROPERTIES
    EXPORT_NAME Server
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)
set_compiler_flags(${target_name})

target_link_libraries(${target_name}
    PRIVATE Cutelyst::Core
)

if (TARGET Cutelyst::EventLoopEPoll)
    target_link_libraries(${target_name}
        PRIVATE Cutelyst::EventLoopEPoll
    )
    target_compile_definitions(${target_name} PRIVATE HAS_EventLoopEPoll)
endif ()

if(ENABLE_LTO)
    set_property(TARGET ${target_name} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

set_property(TARGET ${target_name} PROPERTY PUBLIC_HEADER ${cutelyst_server_HEADERS})
install(TARGETS ${target_name}
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Server COMPONENT devel
)

add_executable(cutelystd main.cpp)

if (USE_MIMALLOC)
    target_link_libraries(cutelystd PRIVATE mimalloc-static)
    target_compile_definitions(cutelystd PRIVATE USE_MIMALLOC)
elseif (JEMALLOC_FOUND)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed")
    target_link_libraries(cutelystd PRIVATE ${JEMALLOC_LIBRARIES})
endif()

if(ENABLE_LTO)
    set_property(TARGET cutelystd PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

target_link_libraries(cutelystd
    PRIVATE
    Qt::Core
    Cutelyst::Core
    Cutelyst::Server
)

if (TARGET Cutelyst::EventLoopEPoll)
target_link_libraries(cutelystd
    PRIVATE Cutelyst::EventLoopEPoll
)
endif ()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQtServer.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

set_target_properties(cutelystd PROPERTIES OUTPUT_NAME cutelystd${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR})
install(TARGETS cutelystd
    RUNTIME DESTINATION bin COMPONENT runtime
)

set(cutelystd_TS_FILES
    i18n/cutelystserver.ts
    i18n/cutelystserver.de.ts
    i18n/cutelystserver.en.ts
)

qt_add_lupdate(cutelystd
    TS_FILES ${cutelystd_TS_FILES}
    SOURCES ${cutelyst_server_SRC} main.cpp
    OPTIONS -no-obsolete -locations none
)

qt_add_lrelease(cutelystd
    TS_FILES ${cutelystd_TS_FILES}
    QM_FILES_OUTPUT_VARIABLE cutelystd_QM_FILES
    OPTIONS -idbased
)

install(FILES ${cutelystd_QM_FILES} DESTINATION ${I18NDIR})
