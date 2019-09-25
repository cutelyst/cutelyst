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
    context.cpp
    context_p.h
    action.cpp
    actionchain.cpp
    actionchain.h
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

set(cutelystqt_HEADERS
    cutelyst_global.h
    paramsmultimap.h
    ParamsMultiMap
    action.h
    Action
    application.h
    Application
    context.h
    Context
    controller.h
    Controller
    componentfactory.h
    component.h
    Component
    dispatcher.h
    Dispatcher
    dispatchtype.h
    DispatchType
    engine.h
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

set(cutelystqt_HEADERS_PRIVATE
    common.h
)

add_library(Cutelyst2Qt5
    ${cutelystqt_SRC}
    ${cutelystqt_HEADERS}
    ${cutelystqt_HEADERS_PRIVATE}
)
add_library(Cutelyst${PROJECT_VERSION_MAJOR}Qt5::Core ALIAS Cutelyst2Qt5)
target_compile_features(Cutelyst2Qt5
  PRIVATE
    cxx_auto_type
  PUBLIC
    cxx_nullptr
)

if (CMAKE_GENERATOR MATCHES "Visual Studio")
  set_property(TARGET Cutelyst2Qt5 PROPERTY DEBUG_POSTFIX "d")
endif()

target_compile_definitions(Cutelyst2Qt5
  PRIVATE
    PLUGINS_PREFER_DEBUG_POSTFIX=$<CONFIG:Debug>
)

target_include_directories(Cutelyst2Qt5 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/cutelyst2-qt5>
)

set_target_properties(Cutelyst2Qt5 PROPERTIES
    EXPORT_NAME Core
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)
target_link_libraries(Cutelyst2Qt5
    Qt5::Core
    Qt5::Network
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5Core.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt5Core.pc
  @ONLY
)

set_property(TARGET Cutelyst2Qt5 PROPERTY PUBLIC_HEADER ${cutelystqt_HEADERS})
install(TARGETS Cutelyst2Qt5
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION bin COMPONENT runtime
    ARCHIVE DESTINATION lib COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst2-qt5/Cutelyst COMPONENT devel
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt5Core.pc
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)


add_subdirectory(Plugins)
add_subdirectory(Actions)
