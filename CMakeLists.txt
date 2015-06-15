# CMakeLists for libcutelyst-qt
project(cutelyst)

cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
cmake_policy(VERSION 3.0)

find_package(PkgConfig REQUIRED)
find_package(Qt5 5.3.0 COMPONENTS
    Core
    Network
)

set(CUTELYST_VERSION_MAJOR  "0")
set(CUTELYST_VERSION_MINOR  "9")
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
add_subdirectory(uwsgiEngine)
add_subdirectory(developerEngine)

# TODO rename to examples
#add_subdirectory(server)

add_subdirectory(cmd)
