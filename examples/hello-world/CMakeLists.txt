# Our sources
file(GLOB_RECURSE hello_SRCS *)

# Create the application
add_executable(cutelyst-hello-world ${hello_SRCS})

# Link to Cutelyst
target_link_libraries(cutelyst-hello-world PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Core
    Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Server
    Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Session
)
