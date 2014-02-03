include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(server_SRCS
    root.cpp
    users.cpp
    main.cpp
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

add_executable(cuteserver
    ${server_SRCS}
)

qt5_use_modules(cuteserver Core Network)
target_link_libraries(cuteserver
    cutelyst-qt5
)

install(TARGETS cuteserver DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
