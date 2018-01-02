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

add_library(Cutelyst2Qt5EventLoopEpoll SHARED
    ${eventloop_epoll_SRC}
    ${eventloop_epoll_HEADERS}
)
add_library(Cutelyst2Qt5::EventLoopEPoll ALIAS Cutelyst2Qt5EventLoopEpoll)

set_target_properties(Cutelyst2Qt5EventLoopEpoll PROPERTIES
    EXPORT_NAME EventLoopEPoll
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst2Qt5EventLoopEpoll
    Qt5::Core
)

install(TARGETS Cutelyst2Qt5EventLoopEpoll EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR})

