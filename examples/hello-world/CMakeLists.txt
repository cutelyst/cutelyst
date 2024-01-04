# Our sources
file(GLOB_RECURSE hello_SRCS *)

# Create the application
add_executable(cutelyst-hello-world ${hello_SRCS})

# Link to Cutelyst
target_link_libraries(cutelyst-hello-world PRIVATE
    Qt::Core
    Qt::Network
    Cutelyst::Core
    Cutelyst::Server
    Cutelyst::Session
)
