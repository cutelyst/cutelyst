# Our sources
file(GLOB_RECURSE hello_SRCS *)

# Create the application
add_executable(cutelyst-hello-world ${hello_SRCS})

# Link to Cutelyst
target_link_libraries(cutelyst-hello-world PRIVATE
    Qt5::Core
    Qt5::Network
    Cutelyst::Core
    Cutelyst::WSGI
    Cutelyst::Session
)

# install the application executable
install(TARGETS cutelyst-hello-world DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
