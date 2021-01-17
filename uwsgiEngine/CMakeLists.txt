find_package(PkgConfig REQUIRED)

# uwsgi.h required if #ifdef UWSGI_UUID
pkg_check_modules(UUID REQUIRED uuid)

find_package(CAP REQUIRED)

set(uswgi_engine_SRC
    bodyuwsgi.cpp
    bodyuwsgi.h
    engineuwsgi.cpp
    engineuwsgi.h
    uwsgiconnection.cpp
    uwsgiconnection.h
    plugin.cpp
)

execute_process(COMMAND ${UWSGI_EXEC_PATH} --dot-h
                OUTPUT_FILE uwsgi.h)

execute_process(COMMAND ${UWSGI_EXEC_PATH} --cflags
                OUTPUT_VARIABLE UWSGI_CFLAGS
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_definitions(${UWSGI_CFLAGS})

add_library(cutelyst MODULE ${uswgi_engine_SRC})
target_link_libraries(cutelyst
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Core
)

if (LINUX)
target_link_libraries(cutelyst
    PRIVATE Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::EventLoopEPoll
)
endif (LINUX)

SET_TARGET_PROPERTIES(cutelyst
    PROPERTIES
    SUFFIX "${PROJECT_VERSION_MAJOR}_qt${QT_VERSION_MAJOR}_plugin.so"
    PREFIX ""
)

install(TARGETS cutelyst
    DESTINATION ${UWSGI_PLUGINS_DIR}
)
