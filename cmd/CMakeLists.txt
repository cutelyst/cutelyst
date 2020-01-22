include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(cutelyst_cmd_SRCS
    helper.cpp
    main.cpp
)

add_executable(cutelyst-skell ${cutelyst_cmd_SRCS})
target_compile_features(cutelyst-skell
  PUBLIC
    cxx_generalized_initializers
)
target_link_libraries(cutelyst-skell
    PRIVATE Qt5::Core
    PRIVATE Qt5::Network
    PRIVATE Cutelyst2Qt5::Core
    PRIVATE Cutelyst2Qt5::WSGI
)
if (LINUX)
target_link_libraries(cutelyst-skell
    PRIVATE Cutelyst2Qt5::EventLoopEPoll
)
endif ()

set_target_properties(cutelyst-skell PROPERTIES OUTPUT_NAME cutelyst2)
install(TARGETS cutelyst-skell DESTINATION ${CMAKE_INSTALL_BINDIR})
