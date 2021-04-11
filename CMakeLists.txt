cmake_minimum_required(VERSION 3.9)

project(cutelyst VERSION 3.0.0 LANGUAGES CXX)

include(GNUInstallDirs)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Network Sql REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} 5.6.0 COMPONENTS Core Network Sql REQUIRED)

set(CUTELYST_API_LEVEL "3")

# CMakeLists for Cutelyst library
set(CMAKE_AUTOMOC ON)

# Include our cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

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
include(CMakeDependentOption)
option(BUILD_SHARED_LIBS "Build in shared lib mode" ON)
option(ENABLE_MAINTAINER_CFLAGS "Enable maintainer CFlags" OFF)
option(BUILD_TESTS "Build the Cutelyst tests" ${BUILD_ALL})
option(BUILD_EXAMPLES "Build the Cutelyst examples" ${BUILD_ALL})
option(BUILD_DOCS "Add the make docs target to build the documentationn. Requires doxygen and dot" ${BUILD_ALL})
cmake_dependent_option(BUILD_DOCS_QUIET "Tell doxygen to be quiet while building the documentation." OFF "BUILD_DOCS" OFF)
cmake_dependent_option(BUILD_DOCS_TIMESTAMP "Tell doxygen to insert a build timestamp on every HTML page." ON "BUILD_DOCS" OFF)

if (BUILD_TESTS)
  enable_testing()
endif()

#
# Plugin options
#
option(BUILD_ALL "Build all available modules/plugins" OFF)
option(PLUGIN_MEMCACHED "Enables the memcached plugin" ${BUILD_ALL})
cmake_dependent_option(PLUGIN_MEMCACHEDSESSIONSTORE "Enables the memcached based session store" ON "PLUGIN_MEMCACHED" OFF)
option(PLUGIN_STATICCOMPRESSED "Enables the StaticCompressed plugin" ${BUILD_ALL})
option(PLUGIN_CSRFPROTECTION "Enables the CSRF protection plugin" ${BUILD_ALL})
option(PLUGIN_VIEW_EMAIL "Enables View::Email plugin" ${BUILD_ALL})
option(PLUGIN_VIEW_GRANTLEE "Enables View::Grantlee plugin" ${BUILD_ALL})
option(PLUGIN_VIEW_CUTELEE "Enables View::Cutelee plugin" ${BUILD_ALL})
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
set (DATADIR "${CMAKE_INSTALL_DATADIR}")
set (PKGDATADIR "${CMAKE_INSTALL_DATADIR}")
set (LIBDIR "${CMAKE_INSTALL_LIBDIR}")
set (PKGLIBDIR "${LIBDIR}/cutelyst")
set (GETTEXT_PACKAGE "cutelyst")
set (LOCALE_DIR "${DATADIR}/locale")
set (VERSION "${CUTELYST_VERSION}")
set (BUILDDIR "${CMAKE_BINARY_DIR}")
set (CUTELYST_PLUGINS_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}-plugins" CACHE PATH "Output directory for cutelyst plugins")
message(STATUS "CUTELYST_PLUGINS_DIR: ${CUTELYST_PLUGINS_DIR}")
set (DOXYGEN_TIMESTAMP "YES" CACHE STRING "Enables or disables the footer timestamp in API documentation. Allowed values: YES or NO")
set (QHG_LOCATION "qhelpgenerator" CACHE FILEPATH "Path to the qhelpgenerator executable")
set (MANDIR "${DATADIR}/man" CACHE PATH "Directory to install man pages")
set (I18NDIR "${CMAKE_INSTALL_FULL_DATADIR}/cutelyst${PROJECT_VERSION_MAJOR}/translations" CACHE PATH "Directory to install translations")

add_definitions("-DLOCALSTATEDIR=\"${LOCALSTATEDIR}\"")

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
    -DQT_DISABLE_DEPRECATED_BEFORE=0x060001
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Cutelyst
    ${CMAKE_CURRENT_BINARY_DIR}
)

# cmake config files
configure_file(${CMAKE_MODULE_PATH}/cutelystqt5-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Config.cmake
    @ONLY
)
configure_file(${CMAKE_MODULE_PATH}/cutelystqt5-config-version.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ConfigVersion.cmake
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}ConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}/
)
install(EXPORT CutelystTargets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}/
    FILE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Targets.cmake
    NAMESPACE Cutelyst::
    COMPONENT Devel
)


add_subdirectory(Cutelyst)

if(UNIX AND NOT (APPLE OR HAIKU))
    set(LINUX TRUE)
endif()

if (LINUX)
    if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
        # FreeBSD isn't Linux, but it gets claimed as Linux, because it is
        # UNIX and not APPLE (see just above).
        #
        # Try to find the EPOLL implementation from libepoll-shim; while
        # epoll.h is found, and timerfd.h is found, the FreeBSD implementation
        # in timerfd is insufficient (missing timerfd_gettime), and besides
        # that there is also no eventfd.h in the FreeBSD shims.
        #
        # So, do the work of finding EPOLL, but don't use that subdirectory.
        # Also, decide that we're not Linux after all.
        find_file(EPOLL_H sys/epoll.h
            HINTS libepoll-shim /usr/local/include/libepoll-shim
        )
        if (EPOLL_H)
            get_filename_component(EPOLL_SYS_DIR ${EPOLL_H} DIRECTORY)
            get_filename_component(EPOLL_DIR ${EPOLL_SYS_DIR} DIRECTORY)
            include_directories(${EPOLL_DIR})
            # add_subdirectory(EventLoopEPoll)
        endif()
        set(LINUX FALSE)
    else()
        # Real Linux
        add_subdirectory(EventLoopEPoll)
    endif()
endif()

add_subdirectory(server)

if (PLUGIN_UWSGI)
    find_program(UWSGI_EXEC_PATH NAMES uwsgi)
    set (UWSGI_EXEC_PATH "uwsgi" CACHE FILEPATH "Path to the uWSGI executable")
    find_program(UWSGI_BIN NAMES uwsgi)
    set (UWSGI_PLUGINS_DIR "${CMAKE_INSTALL_PREFIX}/lib/uwsgi/plugins" CACHE PATH "Output directory for uWSGI plugin")
    add_subdirectory(uwsgiEngine)
    message(STATUS "PLUGIN: uWSGI, enabled.")
else()
    message(STATUS "PLUGIN: uWSGI, disabled.")
endif ()

add_subdirectory(cmd)

add_subdirectory(i18n)

add_subdirectory(dox)

if (BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()

# Adds the make docs target. make docs will create three types of documentation.
# It will create html documentation linked to the online documentation of Qt
# and libstdc++, this will be created into the directory "webdox" in the CMAKE_BINARY_DIR.
# A second html documentation will be created that is linked to local available html
# documentation, this will be created into the directory "htmldox" in the CMAKE_BINARY_DIR.
# The third documentation is a QCH file, that will be created as cutelyst2.qch in the
# CMAKE_BINARY_DIR. The QCH documentation will only be created if qhelpgenerator can be found.
# Linkage to third party documentation will only be available if the appropriate tag files
# can be found.
if (BUILD_DOCS)
    find_package(Doxygen REQUIRED dot)

    # It seems that DOXYGEN_DOT_FOUND is not set correctly if it is required
    # as package component, at least not on cmake 3.10 and 3.16. CMake 3.9
    # introduced an improved FindDoxygen module, so lets set this manually
    # for CMake 3.9 or newer.
    if (CMAKE_VERSION VERSION_GREATER 3.8.2)
        set(DOXYGEN_DOT_FOUND YES)
    endif (CMAKE_VERSION VERSION_GREATER 3.8.2)

    if (DOXYGEN_FOUND AND DOXYGEN_DOT_FOUND)
        message (STATUS "Doxygen and dot found. make docs target is available")

        if (BUILD_DOCS_QUIET)
            set(DOXYGEN_QUIET "YES")
            set(DOXYGEN_WARNINGS "NO")
            set(DOXYGEN_WARN_IF_UNDOCUMENTED "NO")
            set(DOXYGEN_WARN_IF_DOC_ERROR "NO")
        else(BUILD_DOCS_QUIET)
            set(DOXYGEN_QUIET "NO")
            set(DOXYGEN_WARNINGS "YES")
            set(DOXYGEN_WARN_IF_UNDOCUMENTED "YES")
            set(DOXYGEN_WARN_IF_DOC_ERROR "YES")
        endif(BUILD_DOCS_QUIET)

        if (BUILD_DOCS_TIMESTAMP)
            set(DOXYGEN_HTML_TIMESTAMP "YES")
        else (BUILD_DOCS_TIMESTAMP)
            set(DOXYGEN_HTML_TIMESTAMP "NO")
        endif (BUILD_DOCS_TIMESTAMP)

        set(DOXYGEN_PROJECT_NUMBER ${PROJECT_VERSION})
        set(DOXYGEN_STRIP_FROM_PATH ${CMAKE_SOURCE_DIR})
        set(DOXYGEN_IMAGE_PATH ${CMAKE_SOURCE_DIR}/dox)
        set(DOXYGEN_GENERATE_TAGFILE ${CMAKE_BINARY_DIR}/${PROJECT_NAME}${PROJECT_VERSION_MAJOR}.tags)
        set(DOXYGEN_DOTFILE_DIRS ${CMAKE_SOURCE_DIR}/dox)

        # Available from http://gcc.gnu.org/onlinedocs/libstdc++/latest-doxygen/libstdc++.tag
        set (DOCS_STLTAGFILE "${CMAKE_SOURCE_DIR}/scripts/libstdc++.tag" CACHE FILEPATH "Path to the doxygen tag file of the libstdc++")
        set (DOCS_STLLOCALHTMLDIR "${CMAKE_SOURCE_DIR}/scripts/libstdc++" CACHE PATH "Path to the directory containing the libstdc++ HTML doxygen documentation")
        if (EXISTS DOCS_STLTAGFILE)
            list(APPEND ONLINETAGS "${DOCS_STLTAGFILE}=http://gcc.gnu.org/onlinedocs/libstdc++/latest-doxygen/")
            if (IS_DIRECTORY "${DOCS_STLLOCALHTMLDIR}")
                list(APPEND LOCALHTMLTAGS "${DOCS_STLTAGFILE}=${DOCS_STLLOCALHTMLDIR}/")
            endif (IS_DIRECTORY "${DOCS_STLLOCALHTMLDIR}")
            list(APPEND QCHTAGS "${DOCS_STLTAGFILE}=qthelp://org.gnu.libstdcxx/libstdc++/")
        endif (EXISTS DOCS_STLTAGFILE)

        find_program(QMAKE_EXE_PATH NAMES qmake-qt5 qmake5 qmake)
        if (QMAKE_EXE_PATH)
            execute_process(COMMAND ${QMAKE_EXE_PATH} -query QT_INSTALL_DOCS OUTPUT_VARIABLE QT_INSTALL_DOCS OUTPUT_STRIP_TRAILING_WHITESPACE)
            if (NOT IS_DIRECTORY "${QT_INSTALL_DOCS}")
                set(QT_INSTALL_DOCS "/usr/share/qt5/doc")
            endif (NOT IS_DIRECTORY "${QT_INSTALL_DOCS}")
        else(QMAKE_EXE_PATH)
            set(QT_INSTALL_DOCS "/usr/share/qt5/doc")
        endif(QMAKE_EXE_PATH)

        set(QT_ONLINE_DOCS_URL "https://doc.qt.io/qt-5/")
        set(QT_DOCS_COMPONENTS "qtcore" "qtsql" "qtnetwork")

        # Qt normally ships with documentation and tagfiles installed into QT_INSTALL_DOCS directory.
        # If your distribution does not provide the tag files for Qt, yout can get a complete one
        # from http://api.kde.org/kdereview-api/kdereview.tar.gz for example. If you have one single
        # big tag file for complete Qt, use DOCS_QTTAGFILE to set the path to it. If you have the
        # modularized tag files from Qt, use DOCS_QTDOCSDIR to set the path to the directory that contains
        # the tag files. The files will be searched there and in subdirectories named by the Qt component.
        set (DOCS_QTTAGFILE "${CMAKE_SOURCE_DIR}/scripts/qttags.tag" CACHE FILEPATH "Path to a single complete doxygen tag file for Qt")
        set (DOCS_QTDOCSDIR ${QT_INSTALL_DOCS} CACHE PATH "Path to a directory containingthe Qt Documentation")
        if (EXISTS DOCS_QTTAGFILE)
            list(APPEND ONLINETAGS "${DOCS_QTTAGFILE}=${QT_ONLINE_DOCS_URL}")
            foreach(_QTDOCCOMP ${QT_DOCS_COMPONENTS})
                if (IS_DIRECTORY "${DOCS_QTDOCSDIR}/${_QTDOCCOMP}")
                    list(APPEND LOCALHTMLTAGS "${DOCS_QTTAGFILE}=${DOCS_QTDOCSDIR}/${_QTDOCCOMP}/")
                endif (IS_DIRECTORY "${DOCS_QTDOCSDIR}/${_QTDOCCOMP}")
                if (EXISTS "${DOCS_QTDOCSDIR}/${_QTDOCCOMP}.qch")
                    list(APPEND QCHTAGS "${DOCS_QTTAGFILE}=qthelp://org.qt-project.${_QTDOCCOMP}.${Qt5_VERSION_MAJOR}${Qt5_VERSION_MINOR}${Qt5_VERSION_PATCH}/${_QTDOCCOMP}/")
                endif (EXISTS "${DOCS_QTDOCSDIR}/${_QTDOCCOMP}.qch")
            endforeach(_QTDOCCOMP ${QT_DOCS_COMPONENTS})
        else(EXISTS DOCS_QTTAGFILE)
            foreach(_QTDOCCOMP ${QT_DOCS_COMPONENTS})
                find_file(_QTDOCCOMPTAGS "${_QTDOCCOMP}.tags" PATHS "${DOCS_QTDOCSDIR}" PATH_SUFFIXES "${_QTDOCCOMP}" NO_DEFAULT_PATH)
                if (_QTDOCCOMPTAGS)
                    list(APPEND ONLINETAGS "${_QTDOCCOMPTAGS}=${QT_ONLINE_DOCS_URL}")
                    if (IS_DIRECTORY "${DOCS_QTDOCSDIR}/${_QTDOCCOMP}")
                        list(APPEND LOCALHTMLTAGS "${_QTDOCCOMPTAGS}=${DOCS_QTDOCSDIR}/${_QTDOCCOMP}/")
                    endif (IS_DIRECTORY "${DOCS_QTDOCSDIR}/${_QTDOCCOMP}")
                    if (EXISTS "${DOCS_QTDOCSDIR}/${_QTDOCCOMP}.qch")
                        list(APPEND QCHTAGS "${_QTDOCCOMPTAGS}=qthelp://org.qt-project.${_QTDOCCOMP}.${Qt5_VERSION_MAJOR}${Qt5_VERSION_MINOR}${Qt5_VERSION_PATCH}/${_QTDOCCOMP}/")
                    endif (EXISTS "${DOCS_QTDOCSDIR}/${_QTDOCCOMP}.qch")
                endif (_QTDOCCOMPTAGS)
                unset(_QTDOCCOMPTAGS CACHE)
            endforeach(_QTDOCCOMP ${QT_DOCS_COMPONENTS})
        endif(EXISTS DOCS_QTTAGFILE)

        string(REPLACE ";" " " DOXYGEN_TAGFILES "${ONLINETAGS}")
        set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
        set(DOXYGEN_HTML_OUTPUT "webdox")

        configure_file(
            "${CMAKE_MODULE_PATH}/Doxyfile.in"
            "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.web"
            @ONLY)

        set(DOCS_ARGS ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.web")

        string(REPLACE ";" " " DOXYGEN_TAGFILES "${LOCALHTMLTAGS}")
        set(DOXYGEN_HTML_OUTPUT "htmldox")

        configure_file(
            "${CMAKE_MODULE_PATH}/Doxyfile.in"
            "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.html"
            @ONLY)

        set(DOCS_ARGS ${DOCS_ARGS} COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.html")

        find_program(DOXYGEN_QHG_LOCATION NAMES qhelpgenerator-qt5 qhelpgenerator5 qhelpgenerator)
        if(DOXYGEN_QHG_LOCATION)
            message(STATUS "qhelpgenerator found. QCH documentation will be created")
            set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qtdocs")
            string(REPLACE ";" " " DOXYGEN_TAGFILES "${QCHTAGS}")
            set(DOXYGEN_GENERATE_QHP "YES")
            set(DOXYGEN_QHP_NAMESPACE "org.cutelyst.${PROJECT_VERSION_MAJOR}${PROJECT_VERSION_MINOR}${PROJECT_VERSION_PATCH}")
            set(DOXYGEN_QHP_VIRTUAL_FOLDER "cutelyst${PROJECT_VERSION_MAJOR}")
            set(DOXYGEN_QCH_FILE "${CMAKE_BINARY_DIR}/cutelyst${PROJECT_VERSION_MAJOR}.qch")

            configure_file(
                "${CMAKE_MODULE_PATH}/Doxyfile.in"
                "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.qt"
                @ONLY)

            set(DOCS_ARGS ${DOCS_ARGS} COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.qt")
        endif(DOXYGEN_QHG_LOCATION)

        add_custom_target(docs
            ${DOCS_ARGS}
        )
    endif()
endif()

include(CPackConfig)
