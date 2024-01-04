set(target_name cutelyst-embedded-widgets)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Network Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} 5.12.0 COMPONENTS Core Network Widgets REQUIRED)

# Our sources
file(GLOB_RECURSE hello_SRCS *)

set(CMAKE_AUTOUIC ON)

# Create the application
add_executable(${target_name} ${hello_SRCS})

# Link to Cutelyst
target_link_libraries(${target_name} PRIVATE
    Qt::Widgets
    Qt::Network
    Cutelyst::Core
    Cutelyst::Server
)
