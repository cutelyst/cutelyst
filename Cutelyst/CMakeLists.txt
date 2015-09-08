include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

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
    Plugins/View/ViewInterface.h
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

add_definitions(
    -std=c++11
)

add_library(cutelyst-qt5 SHARED
    ${cutelystqt_SRC}
    ${cutelystqt_HEADERS}
    ${cutelystqt_HEADERS_PRIVATE}
)
add_library(CutelystQt5::Core ALIAS cutelyst-qt5)
set_property(TARGET cutelyst-qt5 PROPERTY EXPORT_NAME Core)

set_target_properties(cutelyst-qt5 PROPERTIES
    VERSION ${CUTELYST_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)
target_link_libraries(cutelyst-qt5
    Qt5::Core
    Qt5::Network
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cutelyst-qt5.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/cutelyst-qt5.pc
  @ONLY
)

install(TARGETS cutelyst-qt5 EXPORT CutelystQt5Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cutelyst-qt5.pc
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
install(FILES ${cutelystqt_HEADERS}
        DESTINATION include/cutelyst-qt5/Cutelyst
)

add_subdirectory(Plugins)
add_subdirectory(Actions)
