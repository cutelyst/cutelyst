project(cutelyst)

cmake_minimum_required(VERSION 2.8.12)
if (POLICY CMP0043)
  cmake_policy(SET CMP0043 NEW)
endif()

find_package(Qt5 5.4.0 COMPONENTS
    Core
    Network
)

set(CUTELYST_VERSION_MAJOR  "0")
set(CUTELYST_VERSION_MINOR  "10")
set(CUTELYST_VERSION_PATCH  "0")
set(CUTELYST_VERSION_SUFFIX "${VERSION_SUFFIX}")
set(CUTELYST_VERSION "${CUTELYST_VERSION_MAJOR}.${CUTELYST_VERSION_MINOR}.${CUTELYST_VERSION_PATCH}")

set(CUTELYST_API_LEVEL "0")

# CMakeLists for Cutelyst library
set(CMAKE_AUTOMOC ON)

# Include our cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

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

add_definitions("-DLOCALSTATEDIR=\"${LOCALSTATEDIR}\"")

set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_LIBRARY_ARCHITECTURE}" CACHE PATH "Output directory for libraries")

configure_file(config.h.in ${CMAKE_BINARY_DIR}/config.h)

#
# Custom C flags
#
set (MAINTAINER_CFLAGS "-Werror -Wall -Wcast-align -Wno-uninitialized -Wempty-body -Wformat-security -Wformat -Winit-self")
option (DISABLE_MAINTAINER_CFLAGS "Disable maintainer CFlags" ON)
if (DISABLE_MAINTAINER_CFLAGS)
	set (MAINTAINER_CFLAGS "")
endif (DISABLE_MAINTAINER_CFLAGS)
add_definitions(${MAINTAINER_CFLAGS})

add_definitions(-DQT_NO_KEYWORDS)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_subdirectory(Cutelyst)

find_program(UWSGI_BIN NAMES uwsgi)
add_subdirectory(developerEngine)

if (UWSGI_EXEC_PATH)
    add_subdirectory(uwsgiEngine)
endif ()


# TODO rename to examples
#add_subdirectory(server)

add_subdirectory(cmd)
