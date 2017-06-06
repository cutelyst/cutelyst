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
set(CUTELYST_VERSION_MINOR  "7")
set(CUTELYST_VERSION_PATCH  "1")
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

if (WIN32)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif ()

#
# Options
#
option( ENABLE_MAINTAINER_CFLAGS "Enable maintainer CFlags" OFF )
option( BUILD_TESTS "Build the Cutelyst tests" TRUE )

if (BUILD_TESTS)
  enable_testing()
endif()

# NONE

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX
    "/usr" CACHE PATH "cutelyst default install prefix" FORCE)
endif()

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
set (UWSGI_PLUGINS_DIR "${CMAKE_INSTALL_PREFIX}/lib/uwsgi/plugins" CACHE PATH "Output directory for uWSGI plugin")
find_program(UWSGI_EXEC_PATH NAMES uwsgi)
set (UWSGI_EXEC_PATH "uwsgi" CACHE FILEPATH "Path to the uWSGI executable")
set (CUTELYST_PLUGINS_DIR "${CMAKE_INSTALL_PREFIX}/lib/cutelyst-plugins" CACHE PATH "Output directory for cutelyst plugins")
set (DOXYGEN_TIMESTAMP "YES" CACHE STRING "Enables or disables the footer timestamp in API documentation. Allowed values: YES or NO")
set (QHG_LOCATION "qhelpgenerator" CACHE FILEPATH "Path to the qhelpgenerator executable")

add_definitions("-DLOCALSTATEDIR=\"${LOCALSTATEDIR}\"")

set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_LIBRARY_ARCHITECTURE}" CACHE PATH "Output directory for libraries")

configure_file(config.h.in ${CMAKE_BINARY_DIR}/config.h)

#
# Custom C flags
#
if (ENABLE_MAINTAINER_CFLAGS)
    set (MAINTAINER_CFLAGS "-Werror -Wall -Wcast-align -Wno-uninitialized -Wempty-body -Wformat-security -Wformat -Winit-self")
    add_definitions(${MAINTAINER_CFLAGS}
        -DQT_DEPRECATED_WARNINGS
    )
endif ()

add_definitions(
    -DQT_NO_KEYWORDS
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_ASCII
    -DQT_STRICT_ITERATORS
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_USE_QSTRINGBUILDER
)

add_definitions(
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050800
)

if (NOT WIN32)
add_definitions(
    -std=c++11
)
endif()

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

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

if (LINUX)
    add_subdirectory(EventLoopEPoll)
endif()

add_subdirectory(wsgi)

find_program(UWSGI_BIN NAMES uwsgi)
if (UWSGI_EXEC_PATH)
    add_subdirectory(uwsgiEngine)
endif()

add_subdirectory(cmd)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()

find_package(Doxygen)
if (DOXYGEN_FOUND AND DOXYGEN_DOT_FOUND)
  message (STATUS "Doxygen and dot found. make docs target is available")
  # Add support for a docs target

  # Available from http://gcc.gnu.org/onlinedocs/libstdc++/latest-doxygen/libstdc++.tag
  set(STLTAGFILE "${CMAKE_SOURCE_DIR}/scripts/libstdc++.tag=http://gcc.gnu.org/onlinedocs/libstdc++/latest-doxygen")
  set(QTTAGFILE "${CMAKE_SOURCE_DIR}/scripts/qttags.tag=http://qt.nokia.com/doc/${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}")

  set(TAGFILES "${QTTAGFILE} ${STLTAGFILE}")
  set(DOCS_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
  configure_file(
    "${CMAKE_MODULE_PATH}/Doxyfile.in"
    "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
    @ONLY)

  # Add support for a Qt assistant docs target
  # qttags.tag is too big to distribute with Grantlee. Get it
  # from http://api.kde.org/kdereview-api/kdereview.tar.gz for example.
  if(EXISTS ${CMAKE_SOURCE_DIR}/scripts/qttags.tag)
    set(DOCS_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qtdocs")

    # Available from http://steveire.com/libstdc++-longnames.tag
    # Also recommended: http://www.steveire.com/libstdc++-v3.qch
    set(STLTAGFILE "${CMAKE_SOURCE_DIR}/scripts/libstdc++-longnames.tag=qthelp://org.gnu.libstdcxx/libstdc++-v3/")
    set(QTTAGFILE "${CMAKE_SOURCE_DIR}/scripts/qttags.tag=qthelp://com.trolltech.qt/qdoc/")

    set(TAGFILES "${QTTAGFILE} ${STLTAGFILE}")
    set(GENERATE_QHP "yes")

    configure_file(
      "${CMAKE_MODULE_PATH}/Doxyfile.in"
      "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.qt"
      @ONLY)
  endif()

  set( DOCS_ARGS ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" )
  if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.qt")
    set(DOCS_ARGS ${DOCS_ARGS} COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.qt" )
  endif()

  add_custom_target(docs
    ${DOCS_ARGS}
  )
endif()
