set(cutelystqt_SRC
    utils.cpp
    upload.cpp
    upload_p.h
    multipartformdataparser.cpp
    multipartformdataparser_p.h
    stats.cpp
    stats_p.h
    headers.cpp
    headers_p.h
    request.cpp
    request_p.h
    response.cpp
    response_p.h
    context.cpp
    context_p.h
    action.cpp
    actionchain.cpp
    actionchain_p.h
    action_p.h
    engine.cpp
    engine_p.h
    controller.cpp
    controller_p.h
    dispatchtype.cpp
    dispatchtypechained.cpp
    dispatchtypechained_p.h
    dispatchtypepath.cpp
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
    View
    plugin.h
    Plugin
    utils.h
)

set(cutelystqt_HEADERS_PRIVATE
    common.h
)

add_library(cutelyst-qt5 SHARED
    ${cutelystqt_SRC}
    ${cutelystqt_HEADERS}
    ${cutelystqt_HEADERS_PRIVATE}
)
add_library(CutelystQt5::Core ALIAS cutelyst-qt5)
set_property(TARGET cutelyst-qt5 PROPERTY EXPORT_NAME Core)
target_compile_features(cutelyst-qt5
  PRIVATE
    cxx_auto_type
  PUBLIC
    cxx_nullptr
)

if (CMAKE_GENERATOR MATCHES "Visual Studio")
  set_property(TARGET cutelyst-qt5 PROPERTY DEBUG_POSTFIX "d")
endif()

target_compile_definitions(cutelyst-qt5
  PRIVATE
    PLUGINS_PREFER_DEBUG_POSTFIX=$<CONFIG:Debug>
)

set_target_properties(cutelyst-qt5 PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)
target_link_libraries(cutelyst-qt5
    Qt5::Core
    Qt5::Network
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5Core.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/CutelystQt5Core.pc
  @ONLY
)

install(TARGETS cutelyst-qt5 EXPORT CutelystQt5Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CutelystQt5Core.pc
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
install(FILES ${cutelystqt_HEADERS}
        DESTINATION include/cutelyst-qt5/Cutelyst
)

add_subdirectory(Plugins)
add_subdirectory(Actions)
