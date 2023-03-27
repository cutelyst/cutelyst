cmake_minimum_required(VERSION 3.16)

project(cutelyst
    VERSION 3.7.0
    DESCRIPTION "A C++ Web Framework built on top of Qt, using the simple approach of Catalyst (Perl) framework."
    HOMEPAGE_URL "https://cutelyst.org/"
    LANGUAGES CXX)

include(GNUInstallDirs)
include(FetchContent)

set(CUTELYST_API_LEVEL "3")

# CMakeLists for Cutelyst library
set(CMAKE_AUTOMOC ON)

# Include our cmake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (WIN32)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  if(MSVC)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  endif()
endif ()

#
# Options
#
include(CMakeDependentOption)
option(BUILD_WITH_QT6 "Build against Qt 6" OFF)
option(ENABLE_LTO "Build with Link Time Optimization" OFF)
option(BUILD_SHARED_LIBS "Build in shared lib mode" ON)
option(ENABLE_MAINTAINER_CFLAGS "Enable maintainer CFlags" OFF)
option(BUILD_TESTS "Build the Cutelyst tests" ${BUILD_ALL})
option(BUILD_EXAMPLES "Build the Cutelyst examples" ${BUILD_ALL})
option(BUILD_DOCS "Add the make docs target to build the documentationn. Requires doxygen and dot" ${BUILD_ALL})
cmake_dependent_option(BUILD_DOCS_QUIET "Tell doxygen to be quiet while building the documentation." OFF "BUILD_DOCS" OFF)
cmake_dependent_option(BUILD_DOCS_TIMESTAMP "Tell doxygen to insert a build timestamp on every HTML page." ON "BUILD_DOCS" OFF)

if(BUILD_WITH_QT6)
    find_package(QT NAMES Qt6 COMPONENTS Core Network Sql REQUIRED)
    find_package(Qt6 COMPONENTS Core Network Sql REQUIRED)
else()
    find_package(QT NAMES Qt5 COMPONENTS Core Network Sql REQUIRED)
    find_package(Qt5 COMPONENTS Core Network Sql REQUIRED)
endif()

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

if (ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported() # fatal error if IPO is not supported
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
set (BUILDDIR "${CMAKE_CURRENT_BINARY_DIR}")
set (CUTELYST_PLUGINS_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}-plugins" CACHE PATH "Output directory for cutelyst plugins")
message(STATUS "CUTELYST_PLUGINS_DIR: ${CUTELYST_PLUGINS_DIR}")
set (DOXYGEN_TIMESTAMP "YES" CACHE STRING "Enables or disables the footer timestamp in API documentation. Allowed values: YES or NO")
set (MANDIR "${DATADIR}/man" CACHE PATH "Directory to install man pages")
set (I18NDIR "${CMAKE_INSTALL_FULL_DATADIR}/cutelyst${PROJECT_VERSION_MAJOR}-qt${QT_VERSION_MAJOR}/translations" CACHE PATH "Directory to install translations")

add_definitions("-DLOCALSTATEDIR=\"${LOCALSTATEDIR}\"")

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

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
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050E00
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Cutelyst
    ${CMAKE_CURRENT_BINARY_DIR}
)

# cmake config files
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/cutelystqt5-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}Config.cmake
    @ONLY
)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/cutelystqt5-config-version.cmake.in
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

# CMake 3.25 introduces LINUX and FREEBSD as automatically-set variables,
# but before then need to set them by hand.
if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    set(FREEBSD TRUE)
endif()
if(UNIX AND NOT (APPLE OR HAIKU))
    set(LINUX TRUE)
endif()

if (LINUX OR FREEBSD)
    if(FREEBSD)
        find_package(epoll-shim REQUIRED)
        if (epoll-shim_FOUND)
            add_subdirectory(EventLoopEPoll)
            if(NOT TARGET epoll-shim::epoll-shim)
                message(STATUS "No suitable target for epoll-shim")
            endif()
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

add_subdirectory(dox)

if (BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()

# Adds targets to create docs, namely webdocs, htmldocs, qtdocs and docs - where docs
# is an umbrella target that has dependencies to htmldocs and qtdocs.
# It will create html documentation linked to the online documentation of Qt
# and libstdc++, this will be created into the directory "webdocs" in the CMAKE_BINARY_DIR.
# A second html documentation will be created that is linked to local available html
# documentation, this will be created into the directory "htmldocs" in the CMAKE_BINARY_DIR.
# The third documentation is a QCH file, that will be created as cutelyst3.qch in the
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

        add_custom_target(docs)

        # Project related configuration options
        set(DOXYGEN_REPEAT_BRIEF "NO")
        set(DOXYGEN_STRIP_FROM_PATH ${CMAKE_CURRENT_SOURCE_DIR})
        set(DOXYGEN_QT_AUTOBRIEF "YES")
        set(DOXYGEN_TAB_SIZE "8")
        set(DOXYGEN_ALIASES
            "gr_tag{1}=<tt>{%&nbsp\;\\1&nbsp\;%}</tt>"
            "gr_var{1}=<tt>{{&nbsp\;\\1&nbsp\;}}</tt>"
            "accessors=\\par Accessors:^^"
        )
        set(DOXYGEN_BUILTIN_STL_SUPPORT "YES")
        # Build related configuration options
        set(DOXYGEN_EXTRACT_STATIC "YES")
        set(DOXYGEN_INTERNAL_DOCS "YES")
        set(DOXYGEN_CASE_SENSE_NAMES "YES")
        set(DOXYGEN_SORT_BRIEF_DOCS "YES")
        set(DOXYGEN_SORT_MEMBERS_CTORS_1ST "YES")
        set(DOXYGEN_MAX_INITIALIZER_LINES "0")
        set(DOXYGEN_SHOW_USED_FILES "NO")
        # Configuration options related to warning and progress messages
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
        # Configuration options related to the input files
        set(DOXYGEN_FILE_PATTERNS "*.h" "*.dox" "*.cpp")
        set(DOXYGEN_EXCLUDE_PATTERNS ".*" "*_p*")
        set(DOXYGEN_EXCLUDE_SYMBOLS "d_ptr")
        set(DOXYGEN_IMAGE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/dox)
        # Configuration options related to source browsing
        set(DOXYGEN_SOURCE_BROWSER "YES")
        set(DOXYGEN_REFERENCED_BY_RELATION "YES")
        set(DOXYGEN_REFERENCES_RELATION "YES")
        # Configuration options related to the alphabetical class index
        set(DOXYGEN_ALPHABETICAL_INDEX "NO")
        # Configuration options related to the HTML output
        set(DOXYGEN_SEARCHENGINE "NO")
        # Configuration options related to the preprocessor
        set(DOXYGEN_MACRO_EXPANSION "YES")
        set(DOXYGEN_EXPAND_ONLY_PREDEF "YES")
        set(DOXYGEN_PREDEFINED "Q_QDOC" "Q_COMPILER_INITIALIZER_LISTS" "Q_OS_UNIX" "DOXYGEN_SHOULD_SKIP_THIS")
        set(DOXYGEN_EXPAND_AS_DEFINED "TemplateImpl")
        # Configuration options related to external references
        set(DOXYGEN_GENERATE_TAGFILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}${PROJECT_VERSION_MAJOR}.tags)
        # Configuration options related to the dot tool
        set(DOXYGEN_DOTFILE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/dox)
        set(DOXYGEN_COLLABORATION_GRAPH "NO")
        set(DOXYGEN_MAX_DOT_GRAPH_DEPTH "1000")
        if (DOXYGEN_VERSION VERSION_GREATER_EQUAL 1.9.5)
            set(DOXYGEN_DOT_COMMON_ATTR "fontname=FreeSans,fontsize=10")
            set(DOXYGEN_DOT_EDGE_ATTR "labelfontname=FreeSans,labelfontsize=10")
        else (DOXYGEN_VERSION VERSION_GREATER_EQUAL 1.9.5)
            set(DOXYGEN_DOT_FONTNAME "FreeSans")
            set(DOXYGEN_DOT_FONTSIZE 10)
        endif (DOXYGEN_VERSION VERSION_GREATER_EQUAL 1.9.5)

        # Available from http://gcc.gnu.org/onlinedocs/libstdc++/latest-doxygen/libstdc++.tag
        set (DOCS_STLTAGFILE "${CMAKE_CURRENT_SOURCE_DIR}/scripts/libstdc++.tag" CACHE FILEPATH "Path to the doxygen tag file of the libstdc++")
        set (DOCS_STLLOCALHTMLDIR "${CMAKE_CURRENT_SOURCE_DIR}/scripts/libstdc++" CACHE PATH "Path to the directory containing the libstdc++ HTML doxygen documentation")
        if (EXISTS DOCS_STLTAGFILE)
            set(DOXYGEN_BUILTIN_STL_SUPPORT "NO")
            list(APPEND ONLINETAGS "${DOCS_STLTAGFILE}=http://gcc.gnu.org/onlinedocs/libstdc++/latest-doxygen/")
            if (IS_DIRECTORY "${DOCS_STLLOCALHTMLDIR}")
                list(APPEND LOCALHTMLTAGS "${DOCS_STLTAGFILE}=${DOCS_STLLOCALHTMLDIR}/")
            endif (IS_DIRECTORY "${DOCS_STLLOCALHTMLDIR}")
            list(APPEND QCHTAGS "${DOCS_STLTAGFILE}=qthelp://org.gnu.libstdcxx/libstdc++/")
        endif (EXISTS DOCS_STLTAGFILE)

        find_program(QMAKE_EXE_PATH NAMES qmake-qt${QT_VERSION_MAJOR} qmake${QT_VERSION_MAJOR} qmake)
        if (QMAKE_EXE_PATH)
            execute_process(COMMAND ${QMAKE_EXE_PATH} -query QT_INSTALL_DOCS OUTPUT_VARIABLE QT_INSTALL_DOCS OUTPUT_STRIP_TRAILING_WHITESPACE)
            if (NOT IS_DIRECTORY "${QT_INSTALL_DOCS}")
                set(QT_INSTALL_DOCS "/usr/share/qt${QT_VERSION_MAJOR}/doc")
            endif (NOT IS_DIRECTORY "${QT_INSTALL_DOCS}")
        else(QMAKE_EXE_PATH)
            set(QT_INSTALL_DOCS "/usr/share/qt${QT_VERSION_MAJOR}/doc")
        endif(QMAKE_EXE_PATH)

        set(QT_ONLINE_DOCS_URL "https://doc.qt.io/qt-${QT_VERSION_MAJOR}/")
        set(QT_DOCS_COMPONENTS "qtcore" "qtsql" "qtnetwork")

        # Qt normally ships with documentation and tagfiles installed into QT_INSTALL_DOCS directory.
        # If your distribution does not provide the tag files for Qt, yout can get a complete one
        # from http://api.kde.org/kdereview-api/kdereview.tar.gz for example. If you have one single
        # big tag file for complete Qt, use DOCS_QTTAGFILE to set the path to it. If you have the
        # modularized tag files from Qt, use DOCS_QTDOCSDIR to set the path to the directory that contains
        # the tag files. The files will be searched there and in subdirectories named by the Qt component.
        set (DOCS_QTTAGFILE "${CMAKE_CURRENT_SOURCE_DIR}/scripts/qttags.tag" CACHE FILEPATH "Path to a single complete doxygen tag file for Qt")
        set (DOCS_QTDOCSDIR ${QT_INSTALL_DOCS} CACHE PATH "Path to a directory containingthe Qt Documentation")
        if (EXISTS DOCS_QTTAGFILE)
            list(APPEND ONLINETAGS "${DOCS_QTTAGFILE}=${QT_ONLINE_DOCS_URL}")
            foreach(_QTDOCCOMP ${QT_DOCS_COMPONENTS})
                if (IS_DIRECTORY "${DOCS_QTDOCSDIR}/${_QTDOCCOMP}")
                    list(APPEND LOCALHTMLTAGS "${DOCS_QTTAGFILE}=${DOCS_QTDOCSDIR}/${_QTDOCCOMP}/")
                endif (IS_DIRECTORY "${DOCS_QTDOCSDIR}/${_QTDOCCOMP}")
                if (EXISTS "${DOCS_QTDOCSDIR}/${_QTDOCCOMP}.qch")
                    list(APPEND QCHTAGS "${DOCS_QTTAGFILE}=qthelp://org.qt-project.${_QTDOCCOMP}.${QT_VERSION_MAJOR}${QT_VERSION_MINOR}${QT_VERSION_PATCH}/${_QTDOCCOMP}/")
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
                        list(APPEND QCHTAGS "${_QTDOCCOMPTAGS}=qthelp://org.qt-project.${_QTDOCCOMP}.${QT_VERSION_MAJOR}${QT_VERSION_MINOR}${QT_VERSION_PATCH}/${_QTDOCCOMP}/")
                    endif (EXISTS "${DOCS_QTDOCSDIR}/${_QTDOCCOMP}.qch")
                endif (_QTDOCCOMPTAGS)
                unset(_QTDOCCOMPTAGS CACHE)
            endforeach(_QTDOCCOMP ${QT_DOCS_COMPONENTS})
        endif(EXISTS DOCS_QTTAGFILE)

        set(DOXYGEN_HTML_TIMESTAMP "YES")
        set(DOXYGEN_TAGFILES ${ONLINETAGS})
        set(DOXYGEN_HTML_OUTPUT "webdocs")
        doxygen_add_docs(webdocs
            ${CMAKE_CURRENT_SOURCE_DIR}/dox/Mainpage.dox ${CMAKE_SOURCE_DIR}/Cutelyst ${CMAKE_CURRENT_SOURCE_DIR}/server/server.h
            COMMENT "Generate HTML documentation for online usage"
        )
        message(STATUS "Adding build target webdocs")

        set(DOXYGEN_HTML_TIMESTAMP "NO")
        set(DOXYGEN_TAGFILES ${LOCALHTMLTAGS})
        set(DOXYGEN_HTML_OUTPUT "htmldocs")
        doxygen_add_docs(htmldocs
            ${CMAKE_CURRENT_SOURCE_DIR}/dox/Mainpage.dox ${CMAKE_SOURCE_DIR}/Cutelyst ${CMAKE_CURRENT_SOURCE_DIR}/server/server.h
            COMMENT "Generate HTML documentation for local usage"
        )
        add_dependencies(docs htmldocs)
        message(STATUS "Adding build target htmldocs")

        execute_process(COMMAND ${QMAKE_EXE_PATH} -query QT_INSTALL_LIBEXECS OUTPUT_VARIABLE QT_INSTALL_LIBEXECS OUTPUT_STRIP_TRAILING_WHITESPACE)
        execute_process(COMMAND ${QMAKE_EXE_PATH} -query QT_INSTALL_BINS OUTPUT_VARIABLE QT_INSTALL_BINS OUTPUT_STRIP_TRAILING_WHITESPACE)
        find_program(DOXYGEN_QHG_LOCATION NAMES qhelpgenerator-qt${QT_VERSION_MAJOR} qhelpgenerator${QT_VERSION_MAJOR} qhelpgenerator HINTS ${QT_INSTALL_LIBEXECS} ${QT_INSTALL_BINS})
        if(DOXYGEN_QHG_LOCATION)
            set(DOXYGEN_TAGFILES ${QCHTAGS})
            set(DOXYGEN_HTML_OUTPUT "qtdocs")
            set(DOXYGEN_GENERATE_QHP "YES")
            set(DOXYGEN_QHP_NAMESPACE "org.cutelyst.${PROJECT_VERSION_MAJOR}${PROJECT_VERSION_MINOR}${PROJECT_VERSION_PATCH}")
            set(DOXYGEN_QHP_VIRTUAL_FOLDER "cutelyst${PROJECT_VERSION_MAJOR}")
            set(DOXYGEN_QCH_FILE "${CMAKE_CURRENT_BINARY_DIR}/cutelyst${PROJECT_VERSION_MAJOR}.qch")

            doxygen_add_docs(qtdocs
                ${CMAKE_CURRENT_SOURCE_DIR}/dox/Mainpage.dox ${CMAKE_SOURCE_DIR}/Cutelyst ${CMAKE_CURRENT_SOURCE_DIR}/server/server.h
                COMMENT "Generate Qt QCH documentation for local usage"
            )
            add_dependencies(docs qtdocs)
            message(STATUS "Adding build target qtdocs")
        endif(DOXYGEN_QHG_LOCATION)
    endif()
endif()

include(CPackConfig)
