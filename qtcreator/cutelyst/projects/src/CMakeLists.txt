file(GLOB_RECURSE cutelyst_SRCS *.cpp *.h)

set(cutelyst_SRCS
    ${cutelyst_SRCS}
    ${TEMPLATES_SRC}
)

# Create the application
add_library(${PROJECT_NAME} SHARED ${cutelyst_SRCS})

# Link to Cutelyst
target_link_libraries(${PROJECT_NAME}
    Cutelyst::Core
    Cutelyst::StaticSimple
    Qt5::Core
    Qt5::Network
)

