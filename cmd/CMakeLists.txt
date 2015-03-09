include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(cutelyst_cmd_SRCS
    main.cpp
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

add_executable(cutelyst-skell ${cutelyst_cmd_SRCS})
qt5_use_modules(cutelyst-skell Core Network)

set_target_properties(cutelyst-skell PROPERTIES OUTPUT_NAME cutelyst)
install(TARGETS cutelyst-skell DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)
