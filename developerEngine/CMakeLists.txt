set(httpEngine_SRC
    enginehttp.cpp
    enginehttp_p.h
    childprocess.cpp
    childprocess_p.h
)

add_library(cutelyst-dev-http-qt5 SHARED ${httpEngine_SRC})
set_target_properties(cutelyst-dev-http-qt5 PROPERTIES VERSION ${CUTELYST_VERSION} SOVERSION ${CUTELYST_API_LEVEL})
target_compile_features(cutelyst-dev-http-qt5
  PRIVATE
    cxx_auto_type
  PUBLIC
    cxx_nullptr
)

target_link_libraries(cutelyst-dev-http-qt5
    Qt5::Core
    Qt5::Network
)
