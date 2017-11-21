set(plugin_staticcompressed_SRC
    staticcompressed.cpp
    staticcompressed_p.h
)

set(plugin_staticcompressed_HEADERS
    staticcompressed.h
    StaticCompressed
)

if (PLUGIN_STATICCOMPRESSED_ZOPFLI)
    find_library(ZOPFLI NAMES zopfli libzopfli)
    if (ZOPFLI)
        message(STATUS "PLUGIN: StaticCompressed, use zopfli")
    else (ZOPFLI)
        message(FATAL_ERROR "PLUGIN: StaticCompressed, failed to find zopfli")
    endif (ZOPFLI)
    add_definitions(-DZOPFLI_ENABLED)
    set(ZOPFLI_LIB zopfli)
endif (PLUGIN_STATICCOMPRESSED_ZOPFLI)

if (PLUGIN_STATICCOMPRESSED_BROTLI)
    message(STATUS "PLUGIN: StaticCompressed, enable brotli")
    find_package(PkgConfig REQUIRED)
    pkg_search_module(BROTLI REQUIRED libbrotlienc)
    add_definitions(-DBROTLI_ENABLED)
endif (PLUGIN_STATICCOMPRESSED_BROTLI)

add_library(cutelyst_qt5_plugin_staticcompressed SHARED
    ${plugin_staticcompressed_SRC}
    ${plugin_staticcompressed_HEADERS}
)
add_library(CutelystQt5::StaticCompressed ALIAS cutelyst_qt5_plugin_staticcompressed)
set_property(TARGET cutelyst_qt5_plugin_staticcompressed PROPERTY EXPORT_NAME StaticCompressed)

set_target_properties(cutelyst_qt5_plugin_staticcompressed PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(cutelyst_qt5_plugin_staticcompressed
    PRIVATE cutelyst-qt5
    ${ZOPFLI}
    ${BROTLI_LIBRARIES}
)

install(TARGETS cutelyst_qt5_plugin_staticcompressed EXPORT CutelystQt5Targets DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${plugin_staticcompressed_HEADERS}
    DESTINATION include/cutelyst-qt5/Cutelyst/Plugins/StaticCompressed
    COMPONENT StaticCompressed
)
