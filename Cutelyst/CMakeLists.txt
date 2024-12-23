set(cutelystqt_SRC
    action.cpp
    action_p.h
    actionchain.cpp
    actionchain_p.h
    application.cpp
    application_p.h
    appsettings.cpp
    async.cpp
    component.cpp
    component_p.h
    context.cpp
    context_p.h
    controller.cpp
    controller_p.h
    dispatcher.cpp
    dispatcher_p.h
    dispatchtype.cpp
    dispatchtypechained.cpp
    dispatchtypechained.h
    dispatchtypechained_p.h
    dispatchtypepath.cpp
    dispatchtypepath.h
    dispatchtypepath_p.h
    engine.cpp
    engine_p.h
    enginerequest.cpp
    enginerequest.h
    headers.cpp
    multipartformdataparser.cpp
    multipartformdataparser.h
    multipartformdataparser_p.h
    plugin.cpp
    request.cpp
    request_p.h
    response.cpp
    response_p.h
    stats.cpp
    stats_p.h
    testengine.cpp
    testengine_p.h
    upload.cpp
    upload_p.h
    utils.cpp
    view.cpp
)

set(cutelystqt_HEADERS
    Action
    ActionChain
    AppSettings
    Application
    Component
    ComponentFactory
    Context
    Controller
    CoroContext.h
    DispatchType
    Dispatcher
    Engine
    Headers
    ParamsMultiMap
    Plugin
    Request
    Response
    TestEngine
    Upload
    View
    action.h
    actionchain.h
    application.h
    appsettings.h
    async.h
    component.h
    componentfactory.h
    context.h
    controller.h
    dispatcher.h
    dispatchtype.h
    engine.h
    enginerequest.h
    headers.h
    paramsmultimap.h
    plugin.h
    request.h
    response.h
    stats.h
    testengine.hpp
    upload.h
    utils.h
    view.h
    view_p.h
)

set(cutelystqt_HEADERS_PRIVATE
    common.h
)

set(target_name Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR})
add_library(${target_name}
    ${cutelystqt_SRC}
    ${cutelystqt_HEADERS}
    ${cutelystqt_HEADERS_PRIVATE}
)
add_library(Cutelyst::Core ALIAS Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR})
generate_export_header(${target_name} BASE_NAME CUTELYST)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/cutelyst_export.h
    DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst
)

set_compiler_flags(${target_name})

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
    Qt::Core
    Qt::Network
)

set_property(TARGET Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR} PROPERTY PUBLIC_HEADER ${cutelystqt_HEADERS})
install(TARGETS Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/Cutelyst COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQtCore.pc.in
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

qt_add_lupdate(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}
    TS_FILES ${cutelystcore_TS_FILES}
    OPTIONS -no-obsolete -no-recursive -locations none
)

qt_add_lrelease(Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}
    TS_FILES ${cutelystcore_TS_FILES}
    QM_FILES_OUTPUT_VARIABLE cutelystcore_QM_FILES
    OPTIONS -idbased
)

install(FILES ${cutelystcore_QM_FILES} DESTINATION ${I18NDIR})

add_subdirectory(Plugins)
add_subdirectory(Actions)
add_subdirectory(Server)
