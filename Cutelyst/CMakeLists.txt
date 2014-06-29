include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(cutelystqt_SRC
    upload.cpp
    upload_p.h
    request.cpp
    request_p.h
    response.cpp
    response_p.h
    context.cpp
    context_p.h
    action.cpp
    action_p.h
    engine.cpp
    engine_p.h
    enginehttp.cpp
    enginehttp_p.h
    controller.cpp
    dispatchtype.cpp
    dispatchtypepath.cpp
    dispatcher.cpp
    dispatcher_p.h
    view.cpp
    childprocess.cpp
    childprocess_p.h
    application.cpp
    application_p.h
    Plugin/View/ViewInterface.h
    Plugin/plugin.cpp
    Plugin/session.cpp
    Plugin/staticsimple.cpp
    Plugin/authentication.cpp
    Plugin/authentication_p.h
    Plugin/Authentication/minimal.cpp
    Plugin/Authentication/credentialpassword.cpp
)

set(cutelystqt_HEADERS
    request.h
    response.h
    context.h
    Context
    engine.h
    controller.h
    Controller
    view.h
    application.h
    Application
)

set(cutelystqt_plugin_HEADERS
    Plugin/plugin.h
    Plugin/session.h
    Plugin/Session
    Plugin/staticsimple.h
    Plugin/StaticSimple
    Plugin/authentication.h
)

set(cutelystqt_plugin_authentication_HEADERS
    Plugin/Authentication/minimal.h
    Plugin/Authentication/credentialpassword.h
)

set(cutelystqt_HEADERS_PRIVATE
    common.h
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

add_library(cutelyst-qt5 SHARED ${cutelystqt_SRC} ${cutelystqt_HEADERS} ${cutelystqt_HEADERS_PRIVATE})
set_target_properties(cutelyst-qt5 PROPERTIES VERSION ${CUTELYST_VERSION} SOVERSION ${CUTELYST_API_LEVEL})

qt5_use_modules(cutelyst-qt5 Core Network)

set_property(TARGET cutelyst-qt5 PROPERTY COMPILE_DEFINITIONS CUTELYST_STATIC)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cutelyst-qt5.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/cutelyst-qt5.pc
  @ONLY
)

install(TARGETS cutelyst-qt5 DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cutelyst-qt5.pc
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
install(FILES ${cutelystqt_HEADERS}
        DESTINATION include/cutelyst-qt5/Cutelyst
)
install(FILES ${cutelystqt_plugin_HEADERS}
        DESTINATION include/cutelyst-qt5/Cutelyst/Plugin
)
install(FILES ${cutelystqt_plugin_authentication_HEADERS}
        DESTINATION include/cutelyst-qt5/Cutelyst/Plugin/Authentication
)

add_subdirectory(modules)
add_subdirectory(Plugin)
