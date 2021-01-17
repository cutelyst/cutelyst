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
    PRIVATE Qt${QT_VERSION_MAJOR}::Core
    PRIVATE Qt${QT_VERSION_MAJOR}::Network
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Core
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::WSGI
)
if (LINUX)
target_link_libraries(cutelyst-skell
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::EventLoopEPoll
)
endif ()

set_target_properties(cutelyst-skell PROPERTIES OUTPUT_NAME cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR})
install(TARGETS cutelyst-skell DESTINATION ${CMAKE_INSTALL_BINDIR})
