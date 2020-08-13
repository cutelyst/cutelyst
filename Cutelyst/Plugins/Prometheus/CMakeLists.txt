find_package(prometheus-cpp REQUIRED)

set(plugin_prometheus_SRC
    prometheus.cpp
    prometheus_p.h
    prometheus.h
    prometheus_controller.cpp
    prometheus_controller.h
    prometheus_controller_p.h
    prometheus_standard_metrics.cpp
    prometheus_standard_metrics.h
    prometheus_registry.cpp
    prometheus_registry.h
)

set(plugin_prometheus_HEADERS
    prometheus.h
    Prometheus
    prometheus_controller.h
    prometheus_standard_metrics.h
    prometheus_registry.h
)

add_library(Cutelyst2Qt5Prometheus
    ${plugin_prometheus_SRC}
    ${plugin_prometheus_HEADERS}
)
add_library(Cutelyst2Qt5::Prometheus ALIAS Cutelyst2Qt5Prometheus)

set_target_properties(Cutelyst2Qt5Prometheus PROPERTIES
    EXPORT_NAME Prometheus
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst2Qt5Prometheus
    PRIVATE Cutelyst2Qt5::Core
    prometheus-cpp::core
)

set_property(TARGET Cutelyst2Qt5Prometheus PROPERTY PUBLIC_HEADER ${plugin_prometheus_HEADERS})
install(TARGETS Cutelyst2Qt5Prometheus
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION bin COMPONENT runtime
    ARCHIVE DESTINATION lib COMPONENT devel
    PUBLIC_HEADER DESTINATION include/cutelyst2-qt5/Cutelyst/Plugins/Prometheus COMPONENT devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5Prometheus.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5Prometheus.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5Prometheus.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
