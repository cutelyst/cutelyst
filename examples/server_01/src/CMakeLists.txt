file(GLOB_RECURSE server_01_SRCS *.cpp *.h)

set(server_01_SRCS
    ${server_01_SRCS}
    ${TEMPLATES_SRC}
)

# Create the application
add_library(server_01 SHARED ${server_01_SRCS})

# Link to Cutelyst
target_link_libraries(server_01
    Cutelyst::Core
    Qt5::Core
    Qt5::Network
)

