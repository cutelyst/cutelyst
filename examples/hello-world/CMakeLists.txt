project(HelloWorld)

cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
find_package(Qt5 COMPONENTS
    Core
    Network
)
find_package(Cutelyst2Qt5 2.0.0 REQUIRED)

# Auto generate moc files
set(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Cutelyst2Qt5_INCLUDE_DIR}
)

add_subdirectory(src)
