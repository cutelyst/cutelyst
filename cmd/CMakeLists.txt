include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(cutelyst_cmd_SRCS
    helper.cpp
    main.cpp
)

add_executable(cutelyst-skell ${cutelyst_cmd_SRCS})
if(ENABLE_LTO)
    set_property(TARGET cutelyst-skell PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
target_link_libraries(cutelyst-skell
    PRIVATE Qt::Core
    PRIVATE Qt::Network
    PRIVATE Cutelyst::Core
    PRIVATE Cutelyst::Server
)
if (LINUX)
target_link_libraries(cutelyst-skell
    PRIVATE Cutelyst::EventLoopEPoll
)
endif ()

set_target_properties(cutelyst-skell PROPERTIES OUTPUT_NAME cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR})
install(TARGETS cutelyst-skell DESTINATION ${CMAKE_INSTALL_BINDIR})

set(cutelyst-skell_TS_FILES
    i18n/cutelystcmd.ts
    i18n/cutelystcmd.de.ts
    i18n/cutelystcmd.en.ts
)

qt_add_lupdate(cutelyst-skell
    TS_FILES ${cutelyst-skell_TS_FILES}
    OPTIONS -no-obsolete -locations none
)

qt_add_lrelease(cutelyst-skell
    TS_FILES ${cutelyst-skell_TS_FILES}
    QM_FILES_OUTPUT_VARIABLE cutelyst-skell_QM_FILES
    OPTIONS -idbased
)

install(FILES ${cutelyst-skell_QM_FILES} DESTINATION ${I18NDIR})
