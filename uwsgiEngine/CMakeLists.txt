set(uswgi_engine_SRC
    bodybuffereduwsgi.cpp
    bodyuwsgi.cpp
    coreuwsgi.cpp
    engineuwsgi.cpp
    plugin.cpp
    plugin.c
)

execute_process(COMMAND uwsgi --dot-h
                OUTPUT_FILE uwsgi.h)

execute_process(COMMAND uwsgi --cflags
                OUTPUT_VARIABLE UWSGI_CFLAGS
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 ${UWSGI_CFLAGS}")

add_library(cutelyst SHARED ${uswgi_engine_SRC})
qt5_use_modules(cutelyst Core Network)
target_link_libraries(cutelyst
    cutelyst-qt5
)

SET_TARGET_PROPERTIES(cutelyst
    PROPERTIES
    SUFFIX "_plugin.so"
    PREFIX ""
)

install(TARGETS cutelyst
    DESTINATION ${UWSGI_PLUGINS_DIR}
)
