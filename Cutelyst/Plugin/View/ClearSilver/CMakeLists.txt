include_directories(
    /usr/include/ClearSilver
)

add_definitions(-DQT_PLUGIN)

set(clearsilver_plugin_SRC
    clearsilver.cpp
    clearsilver_p.h
    metadata.json
)

add_library(cutelyst-clearsilver SHARED ${clearsilver_plugin_SRC})
qt5_use_modules(cutelyst-clearsilver Core)
target_link_libraries(cutelyst-clearsilver
    ${ClearSilver_CS}
    ${ClearSilver_Util}
    cutelyst-qt5
)

install(TARGETS cutelyst-clearsilver DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cutelyst-plugins)
