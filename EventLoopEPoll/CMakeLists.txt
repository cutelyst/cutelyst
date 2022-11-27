set(eventloop_epoll_SRC
    timers_p.cpp
    socknot_p.cpp
    eventdispatcher_epoll_p.cpp
    eventdispatcher_epoll.cpp
)

set(eventloop_epoll_HEADERS
    eventdispatcher_epoll_p.h
    eventdispatcher_epoll.h
)

add_library(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}EventLoopEpoll
    ${eventloop_epoll_SRC}
    ${eventloop_epoll_HEADERS}
)
add_library(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::EventLoopEPoll ALIAS Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}EventLoopEpoll)

set_target_properties(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}EventLoopEpoll PROPERTIES
    EXPORT_NAME EventLoopEPoll
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}EventLoopEpoll
    Qt${QT_VERSION_MAJOR}::Core
)
if(TARGET epoll-shim::epoll-shim) # FreeBSD only
    target_link_libraries(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}EventLoopEpoll epoll-shim::epoll-shim)
endif()

install(TARGETS Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}EventLoopEpoll EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR})

