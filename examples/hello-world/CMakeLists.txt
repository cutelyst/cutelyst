# Our sources
file(GLOB_RECURSE hello_SRCS *)

# Create the application
add_executable(cutelyst-hello-world ${hello_SRCS})

# Link to Cutelyst
target_link_libraries(cutelyst-hello-world PRIVATE
    Qt5::Core
    Qt5::Network
    Cutelyst2Qt5::Core
    Cutelyst2Qt5::WSGI
    Cutelyst2Qt5::Session
)

# install the application executable
install(TARGETS cutelyst-hello-world DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
