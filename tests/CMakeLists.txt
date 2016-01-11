project(cutelyst_tests)

find_package(Qt5Test 5.5.0 REQUIRED)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set( CUTELYST_PLUGIN_PATH "${CMAKE_BINARY_DIR}/" )

configure_file(cutelyst_paths.h.cmake ${PROJECT_BINARY_DIR}/cutelyst_paths.h)

set (_rcc_file "${PROJECT_BINARY_DIR}/testresource.qrc")

add_library(template_test_builtins STATIC coverageobject.cpp
  coverageobject.cpp
  ${_testresource_rcc_src}
)
target_link_libraries(template_test_builtins Qt5::Test)

macro(CUTELYST_TEMPLATES_UNIT_TESTS)
  foreach(_testname ${ARGN})
    add_executable(${_testname}_exec
                  ${_testname}.cpp
    )
    add_test(${_testname} ${_testname}_exec )
    target_link_libraries(${_testname}_exec CutelystQt5::Core template_test_builtins)

    set_property(GLOBAL APPEND PROPERTY TEST_COVERAGE "${CMAKE_CURRENT_BINARY_DIR}/${_testname}_exec" )
  endforeach(_testname)
endmacro(CUTELYST_TEMPLATES_UNIT_TESTS)

cutelyst_templates_unit_tests(
  testheaders
)

