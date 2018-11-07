# Our sources
file(GLOB_RECURSE hello_SRCS *)

# Create the application
add_executable(hello ${hello_SRCS})

# Link to Cutelyst
target_link_libraries(hello PRIVATE
    Cutelyst::Core
    Qt5::Core
    Qt5::Network
    Cutelyst::WSGI
    Cutelyst::Session
)

# TODO install to a place where uWSGI Cutelyst plugin searches for
install(TARGETS hello DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
