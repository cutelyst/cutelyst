set(httpEngine_SRC
    wsgi.cpp
    protocol.cpp
    protocolhttp.cpp
    cwsgiengine.cpp
    socket.cpp
    tcpserver.cpp
)

add_library(cutelyst_wsgi_qt5 SHARED ${httpEngine_SRC})

add_library(CutelystQt5::WSGI ALIAS cutelyst_wsgi_qt5)
set_property(TARGET cutelyst_wsgi_qt5 PROPERTY EXPORT_NAME WSGI)

set_target_properties(cutelyst_wsgi_qt5 PROPERTIES
    VERSION ${CUTELYST_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(cutelyst_wsgi_qt5
    PRIVATE cutelyst-qt5
)

install(TARGETS cutelyst_wsgi_qt5 EXPORT CutelystQt5Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})

#install(FILES ${cutelyst_wsgi_HEADERS}
#        DESTINATION include/cutelyst-qt5/Cutelyst/WSGI
#        COMPONENT WSGI
#)

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

install(TARGETS cutelyst-wsgi DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)
