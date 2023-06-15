configure_file(cutelyst_global.h.in cutelyst_global.h @ONLY)

set(cutelystqt_SRC
    utils.cpp
    upload.cpp
    upload_p.h
    multipartformdataparser.cpp
    multipartformdataparser.h
    multipartformdataparser_p.h
    stats.cpp
    stats_p.h
    headers.cpp
    request.cpp
    request_p.h
    response.cpp
    response_p.h
    async.cpp
    context.cpp
    context_p.h
    action.cpp
    actionchain.cpp
    actionchain_p.h
    action_p.h
    enginerequest.cpp
    enginerequest.h
    engine.cpp
    engine_p.h
    controller.cpp
    controller_p.h
    dispatchtype.cpp
    dispatchtypechained.cpp
    dispatchtypechained.h
    dispatchtypechained_p.h
    dispatchtypepath.cpp
    dispatchtypepath.h
    dispatchtypepath_p.h
    dispatcher.cpp
    dispatcher_p.h
    component.cpp
    component_p.h
    view.cpp
    application.cpp
    application_p.h
    plugin.cpp
)

if (${QT_VERSION} VERSION_LESS "6.1.0")
    list(APPEND cutelystqt_SRC
        cookie.cpp
        cookie_p.h
    )
endif (${QT_VERSION} VERSION_LESS "6.1.0")

set(cutelystqt_HEADERS
    paramsmultimap.h
    ParamsMultiMap
    action.h
    Action
    actionchain.h
    ActionChain
    application.h
    Application
    async.h
    context.h
    Context
    controller.h
    Controller
    ComponentFactory
    componentfactory.h
    component.h
    Component
    dispatcher.h
    Dispatcher
    dispatchtype.h
    DispatchType
    engine.h
    enginerequest.h
    Engine
    headers.h
    Headers
    request.h
    Request
    response.h
    Response
    stats.h
    upload.h
    Upload
    view.h
    view_p.h
    View
    plugin.h
    Plugin
    utils.h
)

if (${QT_VERSION} VERSION_LESS "6.1.0")
    list(APPEND cutelystqt_HEADERS
        Cookie
        cookie.h
    )
endif (${QT_VERSION} VERSION_LESS "6.1.0")

set(cutelystqt_HEADERS_PRIVATE
    common.h
)

add_library(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}
    ${cutelystqt_SRC}
    ${cutelystqt_HEADERS}
    ${cutelystqt_HEADERS_PRIVATE}
)
add_library(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Core ALIAS Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR})
target_compile_features(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}
  PRIVATE
    cxx_auto_type
  PUBLIC
    cxx_nullptr
)

if (CMAKE_GENERATOR MATCHES "Visual Studio")
  set_property(TARGET Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR} PROPERTY DEBUG_POSTFIX "d")
endif()

if(ENABLE_LTO)
    set_property(TARGET Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

target_compile_definitions(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}
  PRIVATE
    PLUGINS_PREFER_DEBUG_POSTFIX=$<CONFIG:Debug>
)

target_include_directories(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}>
)

set_target_properties(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR} PROPERTIES
    EXPORT_NAME Core
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)
target_link_libraries(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
)

set_property(TARGET Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR} PROPERTY PUBLIC_HEADER ${cutelystqt_HEADERS})
install(TARGETS Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst COMPONENT devel
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cutelyst_global.h
        DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5Core.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Core.pc
  @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Core.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

set(cutelystcore_TS_FILES
    i18n/cutelystcore.de.ts
    i18n/cutelystcore.en.ts
)

if (${QT_VERSION} VERSION_GREATER "6.2.0")

    qt_add_lupdate(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}
        TS_FILES ${cutelystcore_TS_FILES}
        OPTIONS -no-obsolete -no-recursive -locations none
    )

    qt_add_lrelease(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}
        TS_FILES ${cutelystcore_TS_FILES}
        QM_FILES_OUTPUT_VARIABLE cutelystcore_QM_FILES
    )

else ()

    qt_add_translation(cutelystcore_QM_FILES ${cutelystcore_TS_FILES})
    add_custom_target(cutelystcore_TRANSLATIONS ALL
        DEPENDS ${cutelystcore_QM_FILES}
        SOURCES ${cutelystcore_TS_FILES}
    )

endif ()

install(FILES ${cutelystcore_QM_FILES} DESTINATION ${I18NDIR})

add_subdirectory(Plugins)
add_subdirectory(Actions)
