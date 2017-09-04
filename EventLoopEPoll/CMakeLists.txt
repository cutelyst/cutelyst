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

add_library(cutelyst_qt_eventloop_epoll SHARED
    ${eventloop_epoll_SRC}
    ${eventloop_epoll_HEADERS}
)
add_library(CutelystQt5::EventLoopEPoll ALIAS cutelyst_qt_eventloop_epoll)
set_property(TARGET cutelyst_qt_eventloop_epoll PROPERTY EXPORT_NAME EventLoopEPoll)

set_target_properties(cutelyst_qt_eventloop_epoll PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(cutelyst_qt_eventloop_epoll
    Qt5::Core
)

install(TARGETS cutelyst_qt_eventloop_epoll EXPORT CutelystQt5Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})

