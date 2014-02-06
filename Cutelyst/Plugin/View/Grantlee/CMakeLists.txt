include_directories(
    ${Grantlee_INCLUDES}
)

add_definitions(-DQT_PLUGIN)

set(grantlee_plugin_SRC
    grantleeview.cpp
    grantleeview_p.h
    metadata.json
)

add_library(cutelyst-grantlee SHARED ${grantlee_plugin_SRC})
qt5_use_modules(cutelyst-grantlee Core Network)
target_link_libraries(cutelyst-grantlee
    ${Grantlee_LIBRARIES}
    cutelyst-qt5
)

install(TARGETS cutelyst-grantlee DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cutelyst-plugins)
