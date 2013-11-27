include_directories(${CMAKE_BINARY_DIR}
		    ${CMAKE_CURRENT_BINARY_DIR}
		    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(cutelystqt_SRC
    cutelystrequest.cpp
    cutelystrequest_p.h
    cutelystresponse.cpp
    cutelystresponse_p.h
    cutelyst.cpp
    cutelyst_p.h
    cutelystaction.cpp
    cutelystengine.cpp
    cutelystengine_p.h
    #cutelystengineuwsgi.cpp
    cutelystenginehttp.cpp
    cutelystenginehttp_p.h
    cutelystcontroller.cpp
    cutelystdispatchtype.cpp
    cutelystdispatchtypepath.cpp
    cutelystdispatchtypeindex.cpp
    cutelystdispatchtypedefault.cpp
    cutelystdispatcher.cpp
    cutelystdispatcher_p.h
    cutelystchildprocess.cpp
    cutelystchildprocess_p.h
    cutelystplugin.cpp
    cutelystapplication.cpp
    cutelystapplication_p.h
    cutelystpluginsession.cpp
    cpstaticsimple.cpp
    uwsgi.h
)

set(cutelystqt_HEADERS
    cutelystrequest.h
    cutelystresponse.h
    cutelyst.h
    Cutelyst
    cutelystengine.h
    cutelystcontroller.h
    CutelystController
    cutelystapplication.h
    CutelystApplication
)

set(cutelystqt_HEADERS_PRIVATE
    common.h
)

qt5_wrap_cpp(cutelystqt_MOC_SRC ${cutelystqt_HEADERS} ${cutelystqt_HEADERS_PRIVATE})

add_library(cutelyst-qt5 SHARED ${cutelystqt_SRC} ${cutelystqt_HEADERS} ${cutelystqt_HEADERS_PRIVATE})
set_target_properties(cutelyst-qt5 PROPERTIES VERSION ${CUTELYST_VERSION} SOVERSION ${CUTELYST_API_LEVEL})

qt5_use_modules(cutelyst-qt5 Core Network DBus)
target_link_libraries(cutelyst-qt5

)

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
        DESTINATION include/CutelystQt5
)

add_subdirectory(modules)
