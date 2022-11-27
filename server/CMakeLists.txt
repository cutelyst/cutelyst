option(USE_JEMALLOC "Use jemalloc memory allocator" ${BUILD_ALL})
if (USE_JEMALLOC)
    find_package(JeMalloc REQUIRED)
endif ()

option(USE_MIMALLOC "Use mimalloc memory allocator" ${BUILD_ALL})
if (USE_MIMALLOC)
  FetchContent_Declare(
    mimalloc
    GIT_REPOSITORY https://github.com/microsoft/mimalloc.git
    GIT_TAG        b3b0fb58326a96beea1f4872bc1489d1d9fda7e0 # v2.0.2
  )
  set(MI_BUILD_SHARED OFF)
  set(MI_BUILD_TESTS OFF)
  set(MI_BUILD_OBJECT OFF)
  FetchContent_MakeAvailable(mimalloc)
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

set(target_name_server Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Server)

add_library(${target_name_server} ${cutelyst_server_SRC})

add_library(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Server ALIAS ${target_name_server})

set_target_properties(${target_name_server} PROPERTIES
    EXPORT_NAME Server
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(${target_name_server}
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Core
)

if (TARGET Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::EventLoopEPoll)
target_link_libraries(${target_name_server}
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::EventLoopEPoll
)
target_compile_definitions(${target_name_server} PRIVATE HAS_EventLoopEPoll)
endif ()

if(ENABLE_LTO)
    set_property(TARGET ${target_name_server} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

set_property(TARGET ${target_name_server} PROPERTY PUBLIC_HEADER ${cutelyst_server_HEADERS})
install(TARGETS ${target_name_server}
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst/Server COMPONENT devel
)

add_executable(cutelystd main.cpp)

if (JEMALLOC_FOUND)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed")
    target_link_libraries(cutelystd PRIVATE ${JEMALLOC_LIBRARIES})
endif()

if (USE_MIMALLOC)
    target_link_libraries(cutelystd PRIVATE mimalloc-static)
endif()

if(ENABLE_LTO)
    set_property(TARGET cutelystd PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

target_link_libraries(cutelystd
    PRIVATE Qt${QT_VERSION_MAJOR}::Core
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Core
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Server
)

if (TARGET Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::EventLoopEPoll)
target_link_libraries(cutelystd
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::EventLoopEPoll
)
endif ()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5Server.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/${target_name_server}.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${target_name_server}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

set_target_properties(cutelystd PROPERTIES OUTPUT_NAME cutelystd${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR})
install(TARGETS cutelystd
    RUNTIME DESTINATION bin COMPONENT runtime
)

set(cutelystd_TS_FILES
    i18n/cutelystserver.de.ts
    i18n/cutelystserver.en.ts
)

if (${QT_VERSION} VERSION_GREATER "6.2.0")

    qt_add_lupdate(cutelystd
        TS_FILES ${cutelystd_TS_FILES}
        SOURCES ${cutelyst_server_SRC} main.cpp
        OPTIONS -no-obsolete -locations none
    )

    qt_add_lrelease(cutelystd
        TS_FILES ${cutelystd_TS_FILES}
        QM_FILES_OUTPUT_VARIABLE cutelystd_QM_FILES
    )

else ()

    qt_add_translation(cutelystd_QM_FILES ${cutelystd_TS_FILES})
    add_custom_target(cutelystd_TRANSLATIONS ALL
        DEPENDS ${cutelystd_QM_FILES}
        SOURCES ${cutelystd_TS_FILES}
    )

endif ()

install(FILES ${cutelystd_QM_FILES} DESTINATION ${I18NDIR})
