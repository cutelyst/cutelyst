# Our sources
file(GLOB_RECURSE hello_SRCS *)

# Create the application
add_library(hello SHARED ${hello_SRCS})

# Link to Cutelyst
target_link_libraries(hello
    Cutelyst::Core
    Cutelyst::Session
)

# TODO install to a place where uWSGI Cutelyst plugin searches for
install(TARGETS hello DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
