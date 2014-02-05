set(uswgi_engine_SRC
    engineuwsgi.cpp
    plugin.c
)

execute_process(COMMAND uwsgi --dot-h
                OUTPUT_FILE uwsgi.h)

execute_process(COMMAND uwsgi --cflags
                OUTPUT_VARIABLE UWSGI_CFLAGS
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 ${UWSGI_CFLAGS}")

add_library(cutelyst_uwsgi_plugin SHARED ${uswgi_engine_SRC})
qt5_use_modules(cutelyst_uwsgi_plugin Core Network)
target_link_libraries(cutelyst_uwsgi_plugin
    cutelyst-qt5
)

install(TARGETS cutelyst_uwsgi_plugin DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cutelyst-plugins)
