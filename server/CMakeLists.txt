set(CMAKE_AUTOMOC ON)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(server_SRCS
    root.cpp
    main.cpp
)

add_executable(cuteserver
    ${server_SRCS}
)

qt5_use_modules(cuteserver Core Network DBus)
target_link_libraries(cuteserver
    cutelyst-qt5
)

install(TARGETS cuteserver DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
