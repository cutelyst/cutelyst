cmake_minimum_required(VERSION 3.1)
cmake_policy(SET CMP0048 NEW)

if (POLICY CMP0043)
  cmake_policy(SET CMP0043 NEW)
endif()
if (POLICY CMP0063)
  cmake_policy(SET CMP0063 NEW)
endif()

project(cutelyst VERSION 2.1.0)

find_package(Qt5 5.6.0 REQUIRED COMPONENTS
    Core
    Network
    Sql
)

set(CUTELYST_API_LEVEL "2")

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
  if(MSVC)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  endif()
endif ()

#
# Options
#
option( ENABLE_MAINTAINER_CFLAGS "Enable maintainer CFlags" OFF )
option( BUILD_TESTS "Build the Cutelyst tests" ${BUILD_ALL} )

if (BUILD_TESTS)
  enable_testing()
endif()

#
# Plugin options
#
include(CMakeDependentOption)
option(BUILD_ALL "Build all available modules/plugins" OFF)
option(PLUGIN_MEMCACHED "Enables the memcached plugin" ${BUILD_ALL})
cmake_dependent_option(PLUGIN_MEMCACHEDSESSIONSTORE "Enables the memcached based session store" ON "PLUGIN_MEMCACHED" OFF)
option(PLUGIN_STATICCOMPRESSED "Enables the StaticCompressed plugin" ${BUILD_ALL})
option(PLUGIN_CSRFPROTECTION "Enables the CSRF protection plugin" ${BUILD_ALL})
option(PLUGIN_VIEW_EMAIL "Enables View::Email plugin" ${BUILD_ALL})
option(PLUGIN_VIEW_GRANTLEE "Enables View::Grantlee plugin" ${BUILD_ALL})
option(PLUGIN_VIEW_CLEARSILVER "Enables View::ClearSilver plugin" ${BUILD_ALL})
option(PLUGIN_UWSGI "Enables uWSGI plugin" ${BUILD_ALL})
option(PLUGIN_VALIDATOR_PWQUALITY "Enables ValidatorPwQuality that requires libpwquality 1.2.2 or newer" ${BUILD_ALL})

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
set (CUTELYST_PLUGINS_DIR "${CMAKE_INSTALL_PREFIX}/lib/cutelyst2-plugins" CACHE PATH "Output directory for cutelyst plugins")
set (DOXYGEN_TIMESTAMP "YES" CACHE STRING "Enables or disables the footer timestamp in API documentation. Allowed values: YES or NO")
set (QHG_LOCATION "qhelpgenerator" CACHE FILEPATH "Path to the qhelpgenerator executable")
set (MANDIR "${DATADIR}/man" CACHE PATH "Directory to install man pages")
set (I18NDIR "${DATADIR}/cutelyst${PROJECT_VERSION_MAJOR}/translations" CACHE PATH "Directory to install translations")

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
    -DQT_NO_SIGNALS_SLOTS_KEYWORDS
    -DQT_USE_FAST_OPERATOR_PLUS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050900
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Cutelyst
    ${CMAKE_CURRENT_BINARY_DIR}
)

# cmake config files
configure_file(${CMAKE_MODULE_PATH}/cutelystqt5-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt5Config.cmake
    @ONLY
)
configure_file(${CMAKE_MODULE_PATH}/cutelystqt5-config-version.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt5ConfigVersion.cmake
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt5Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt5ConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Cutelyst${PROJECT_VERSION_MAJOR}Qt5/
)
install(EXPORT CutelystTargets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Cutelyst${PROJECT_VERSION_MAJOR}Qt5/
    FILE Cutelyst${PROJECT_VERSION_MAJOR}Qt5Targets.cmake
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

if (PLUGIN_UWSGI)
    find_program(UWSGI_EXEC_PATH NAMES uwsgi)
    set (UWSGI_EXEC_PATH "uwsgi" CACHE FILEPATH "Path to the uWSGI executable")
    find_program(UWSGI_BIN NAMES uwsgi)
    set (UWSGI_PLUGINS_DIR "${CMAKE_INSTALL_PREFIX}/lib/uwsgi/plugins" CACHE PATH "Output directory for uWSGI plugin")
    add_subdirectory(uwsgiEngine)
    message(STATUS "PLUGIN: uWSGI, enabled.")
endif ()

add_subdirectory(cmd)

add_subdirectory(i18n)

add_subdirectory(dox)

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
