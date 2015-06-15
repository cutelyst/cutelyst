include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(httpEngine_SRC
    enginehttp.cpp
    enginehttp_p.h
    childprocess.cpp
    childprocess_p.h
)

add_definitions(
    -std=c++11
)

add_library(cutelyst-dev-http-qt5 SHARED ${httpEngine_SRC})
set_target_properties(cutelyst-dev-http-qt5 PROPERTIES VERSION ${CUTELYST_VERSION} SOVERSION ${CUTELYST_API_LEVEL})

qt5_use_modules(cutelyst-dev-http-qt5 Core Network)
