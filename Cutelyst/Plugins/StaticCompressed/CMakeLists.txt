cmake_dependent_option(PLUGIN_STATICCOMPRESSED_ZOPFLI "Enables the use of zofpli instead of zlib for gzip compression" OFF "PLUGIN_STATICCOMPRESSED" OFF)
cmake_dependent_option(PLUGIN_STATICCOMPRESSED_BROTLI "Enables the support of the brotli compression format" OFF "PLUGIN_STATICCOMPRESSED" OFF)

set(plugin_staticcompressed_SRC
    staticcompressed.cpp
    staticcompressed_p.h
)

set(plugin_staticcompressed_HEADERS
    staticcompressed.h
    StaticCompressed
)

add_library(Cutelyst2Qt5StaticCompressed
    ${plugin_staticcompressed_SRC}
    ${plugin_staticcompressed_HEADERS}
)
add_library(Cutelyst2Qt5::StaticCompressed ALIAS Cutelyst2Qt5StaticCompressed)

set_target_properties(Cutelyst2Qt5StaticCompressed PROPERTIES
    EXPORT_NAME StaticCompressed
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst2Qt5StaticCompressed
    PUBLIC
        Cutelyst2Qt5::Core
)

# used in the pkg-config file
set(CUTELYST_STATICCOMPRESSED_DEFINES "")

if (PLUGIN_STATICCOMPRESSED_ZOPFLI)
    find_package(Zopfli REQUIRED)
    message(STATUS "PLUGIN: StaticCompressed, enable zopfli")
    target_link_libraries(Cutelyst2Qt5StaticCompressed
        PRIVATE
            ${ZOPFLI_LIBRARIES}
    )
    target_compile_definitions(Cutelyst2Qt5StaticCompressed
        PUBLIC
            CUTELYST_STATICCOMPRESSED_WITH_ZOPFLI
    )
    set(CUTELYST_STATICCOMPRESSED_DEFINES "${CUTELYST_STATICCOMPRESSED_DEFINES} -DCUTELYST_STATICCOMPRESSED_WITH_ZOPFLI")
endif (PLUGIN_STATICCOMPRESSED_ZOPFLI)

if (PLUGIN_STATICCOMPRESSED_BROTLI)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(BROTLI REQUIRED libbrotlienc)
    message(STATUS "PLUGIN: StaticCompressed, enable brotli")
    target_link_libraries(Cutelyst2Qt5StaticCompressed
        PRIVATE
            ${BROTLI_LIBRARIES}
    )
    target_compile_definitions(Cutelyst2Qt5StaticCompressed
        PUBLIC
            CUTELYST_STATICCOMPRESSED_WITH_BROTLI
    )
    set(CUTELYST_STATICCOMPRESSED_DEFINES "${CUTELYST_STATICCOMPRESSED_DEFINES} -DCUTELYST_STATICCOMPRESSED_WITH_BROTLI")
endif (PLUGIN_STATICCOMPRESSED_BROTLI)

set_property(TARGET Cutelyst2Qt5StaticCompressed PROPERTY PUBLIC_HEADER ${plugin_staticcompressed_HEADERS})
install(TARGETS Cutelyst2Qt5StaticCompressed
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst2-qt5/Cutelyst/Plugins/StaticCompressed COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5StaticCompressed.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5StaticCompressed.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5StaticCompressed.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

