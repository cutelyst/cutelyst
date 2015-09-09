find_package(PkgConfig REQUIRED)

# uwsgi.h required if #ifdef UWSGI_UUID
pkg_check_modules(UUID REQUIRED uuid)

find_package(CAP REQUIRED)

set(uswgi_engine_SRC
    bodybuffereduwsgi.cpp
    bodyuwsgi.cpp
    engineuwsgi.cpp
    plugin.cpp
)

execute_process(COMMAND ${UWSGI_EXEC_PATH} --dot-h
                OUTPUT_FILE uwsgi.h)

execute_process(COMMAND ${UWSGI_EXEC_PATH} --cflags
                OUTPUT_VARIABLE UWSGI_CFLAGS
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_definitions(${UWSGI_CFLAGS})

add_library(cutelyst SHARED ${uswgi_engine_SRC})
target_link_libraries(cutelyst
    cutelyst-qt5
    Qt5::Core
    Qt5::Network
)

SET_TARGET_PROPERTIES(cutelyst
    PROPERTIES
    SUFFIX "_plugin.so"
    PREFIX ""
)

install(TARGETS cutelyst
    DESTINATION ${UWSGI_PLUGINS_DIR}
)
