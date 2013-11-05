# CMakeLists for Cutelyst library
set(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# We need add -DQT_WIDGETS_LIB when using QtWidgets in Qt 5.
add_definitions(${Qt5Core_DEFINITIONS})
add_definitions(-fPIC)

include_directories(${CMAKE_BINARY_DIR}
		    ${CMAKE_CURRENT_BINARY_DIR}
		    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(cutelystqt_SRC
    cutelystrequest.cpp
    cutelystrequest_p.h
    cutelystresponse.cpp
    cutelystcontext.cpp
    cutelystaction.cpp
    cutelystengine.cpp
    cutelystengine_p.h
    cutelystenginehttp.cpp
    cutelystcontroller.cpp
    cutelystdispatchtype.cpp
    cutelystdispatchtypepath.cpp
    cutelystdispatcher.cpp
    cutelystchildprocess.cpp
    cutelystchildprocess_p.h
    cutelystapplication.cpp
    cutelystapplication_p.h
)

set(cutelystqt_HEADERS
    cutelystrequest.h
    cutelystresponse.h
    cutelystcontext.h
    cutelystengine.h
    cutelystcontroller.h
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
target_link_libraries(cutelyst-qt5)

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
