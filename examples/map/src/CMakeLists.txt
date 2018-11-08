file(GLOB_RECURSE map_SRCS *.cpp *.h)

set(map_SRCS
    ${map_SRCS}
    ${TEMPLATES_SRC}
)

# Create the application
add_executable(map ${map_SRCS})

# Link to Cutelyst
target_link_libraries(map PRIVATE
    Cutelyst::Core
    Qt5::Core
    Qt5::Network
    Cutelyst::WSGI
)

