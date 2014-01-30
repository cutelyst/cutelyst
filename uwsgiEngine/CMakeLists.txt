#add_definitions(-DQT_PLUGIN)

set(uswgi_engine_SRC
    engineuwsgi.cpp
    plugin.c
)

add_library(cutelyst_uwsgi_plugin SHARED ${uswgi_engine_SRC})
qt5_use_modules(cutelyst_uwsgi_plugin Core Network)
target_link_libraries(cutelyst_uwsgi_plugin
    cutelyst-qt5
)

install(TARGETS cutelyst_uwsgi_plugin DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cutelyst-plugins)
