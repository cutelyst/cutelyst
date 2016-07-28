set(httpEngine_SRC
    wsgi.cpp
    protocol.cpp
    protocolhttp.cpp
    cuteengine.cpp
    socket.cpp
    tcpserver.cpp
)

add_library(cutelyst_wsgi_qt5 SHARED ${httpEngine_SRC})
set_target_properties(cutelyst_wsgi_qt5 PROPERTIES VERSION ${CUTELYST_VERSION} SOVERSION ${CUTELYST_API_LEVEL})
target_link_libraries(cutelyst_wsgi_qt5
    Qt5::Core
    Qt5::Network
#    eventdispatcher_epoll
)

set(cutelyst_wsgi_SRCS
    main.cpp
)

add_executable(cutelyst-wsgi ${cutelyst_wsgi_SRCS} ${httpEngine_SRC})
target_compile_features(cutelyst-wsgi
  PUBLIC
    cxx_generalized_initializers
)
target_link_libraries(cutelyst-wsgi
    cutelyst-qt5
    cutelyst_wsgi_qt5
    Qt5::Core
    Qt5::Network
)

install(TARGETS cutelyst-wsgi DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)
