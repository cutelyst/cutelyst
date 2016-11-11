project(cutelyst)

cmake_minimum_required(VERSION 3.1)
if (POLICY CMP0043)
  cmake_policy(SET CMP0043 NEW)
endif()
if (POLICY CMP0063)
  cmake_policy(SET CMP0063 NEW)
endif()

find_package(Qt5 5.6.0 COMPONENTS
    Core
    Network
    Sql
)

set(CUTELYST_VERSION_MAJOR  "1")
set(CUTELYST_VERSION_MINOR  "1")
set(CUTELYST_VERSION_PATCH  "0")
set(CUTELYST_VERSION_SUFFIX "${VERSION_SUFFIX}")
set(CUTELYST_VERSION "${CUTELYST_VERSION_MAJOR}.${CUTELYST_VERSION_MINOR}.${CUTELYST_VERSION_PATCH}")

set(CUTELYST_API_LEVEL "0")

# CMakeLists for Cutelyst library
set(CMAKE_AUTOMOC ON)

# Include our cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Forbid in-tree building
if(${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})
      message(STATUS "Please do an out-of-tree build:")
      message(STATUS "rm -f CMakeCache.txt && mkdir build && cd build; cmake .. && make")
      message(FATAL_ERROR "In-tree-build detected!")
endif(${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})

#
# Options
#
option( DISABLE_MAINTAINER_CFLAGS "Disable maintainer CFlags" ON )
option( BUILD_TESTS "Build the Cutelyst tests" TRUE )

if (BUILD_TESTS)
  enable_testing()
endif()

# NONE

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX
    "/usr" CACHE PATH "cutelyst default install prefix" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

#
# Configure files
#
set (PREFIXDIR "${CMAKE_INSTALL_PREFIX}")
set (CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${LIBNAME}/")
set (DATADIR "${CMAKE_INSTALL_PREFIX}/share")
set (PKGDATADIR "${DATA_INSTALL_DIR}")
set (LIBDIR "${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}")
set (PKGLIBDIR "${LIBDIR}/cutelyst")
set (GETTEXT_PACKAGE "cutelyst")
set (LOCALE_DIR "${DATADIR}/locale")
set (VERSION "${CUTELYST_VERSION}")
set (BUILDDIR "${CMAKE_BINARY_DIR}")
set (UWSGI_PLUGINS_DIR "/usr/lib/uwsgi/plugins" CACHE PATH "Output directory for uWSGI plugin")
find_program(UWSGI_EXEC_PATH NAMES uwsgi)
set (UWSGI_EXEC_PATH "uwsgi" CACHE FILEPATH "Path to the uWSGI executable")
set (CUTELYST_PLUGINS_DIR "${CMAKE_INSTALL_PREFIX}/lib/cutelyst-plugins" CACHE PATH "Output directory for cutelyst plugins")

add_definitions("-DLOCALSTATEDIR=\"${LOCALSTATEDIR}\"")

set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_LIBRARY_ARCHITECTURE}" CACHE PATH "Output directory for libraries")

configure_file(config.h.in ${CMAKE_BINARY_DIR}/config.h)

#
# Custom C flags
#
set (MAINTAINER_CFLAGS "-Werror -Wall -Wcast-align -Wno-uninitialized -Wempty-body -Wformat-security -Wformat -Winit-self")
if (DISABLE_MAINTAINER_CFLAGS)
	set (MAINTAINER_CFLAGS "")
endif (DISABLE_MAINTAINER_CFLAGS)
add_definitions(${MAINTAINER_CFLAGS})

add_definitions(
    -DQT_NO_KEYWORDS
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_ASCII
    -DQT_STRICT_ITERATORS
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_USE_QSTRINGBUILDER
)

if (NOT WIN32)
add_definitions(
    -std=c++11
)
endif (NOT WIN32)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Cutelyst
    ${CMAKE_CURRENT_BINARY_DIR}
)

# cmake config files
configure_file(${CMAKE_MODULE_PATH}/cutelystqt5-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cutelystqt5-config.cmake
    @ONLY
)
configure_file(${CMAKE_MODULE_PATH}/cutelystqt5-config-version.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cutelystqt5-config-version.cmake
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cutelystqt5-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cutelystqt5-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cutelystqt5/
)
install(EXPORT CutelystQt5Targets
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/cutelystqt5/"
    FILE CutelystQt5Targets.cmake
    NAMESPACE Cutelyst::
    COMPONENT Devel
)


add_subdirectory(Cutelyst)
add_subdirectory(wsgi)

find_program(UWSGI_BIN NAMES uwsgi)
if (UWSGI_EXEC_PATH)
    add_subdirectory(uwsgiEngine)
endif (UWSGI_EXEC_PATH)

add_subdirectory(cmd)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
