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
target_compile_features(cutelyst-skell
  PUBLIC
    cxx_generalized_initializers
)
target_link_libraries(cutelyst-skell
    PRIVATE Qt${QT_VERSION_MAJOR}::Core
    PRIVATE Qt${QT_VERSION_MAJOR}::Network
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Core
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Server
)
if (LINUX)
target_link_libraries(cutelyst-skell
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::EventLoopEPoll
)
endif ()

set_target_properties(cutelyst-skell PROPERTIES OUTPUT_NAME cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR})
install(TARGETS cutelyst-skell DESTINATION ${CMAKE_INSTALL_BINDIR})

set(cutelyst-skell_TS_FILES
    i18n/cutelystcmd.de.ts
    i18n/cutelystcmd.en.ts
)

if (${QT_VERSION} VERSION_GREATER "6.2.0")

    qt_add_lupdate(cutelyst-skell
        TS_FILES ${cutelyst-skell_TS_FILES}
        OPTIONS -no-obsolete -locations none
    )

    qt_add_lrelease(cutelyst-skell
        TS_FILES ${cutelyst-skell_TS_FILES}
        QM_FILES_OUTPUT_VARIABLE cutelyst-skell_QM_FILES
    )

else ()

    qt_add_translation(cutelyst-skell_QM_FILES ${cutelyst-skell_TS_FILES})
    add_custom_target(cutelyst-skell_TRANSLATIONS ALL
        DEPENDS ${cutelyst-skell_QM_FILES}
        SOURCES ${cutelyst-skell_TS_FILES}
    )

endif ()

install(FILES ${cutelyst-skell_QM_FILES} DESTINATION ${I18NDIR})
