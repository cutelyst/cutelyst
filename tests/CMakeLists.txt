project(cutelyst_tests)

find_package(Qt5Test 5.6.0 REQUIRED)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set( CUTELYST_PLUGIN_PATH "${CMAKE_BINARY_DIR}/" )

configure_file(cutelyst_paths.h.cmake ${PROJECT_BINARY_DIR}/cutelyst_paths.h)

set (_rcc_file "${PROJECT_BINARY_DIR}/testresource.qrc")

add_library(coverage_test STATIC coverageobject.cpp
  coverageobject.cpp
  ${_testresource_rcc_src}
)
target_compile_features(coverage_test
  PRIVATE
    cxx_auto_type
  PUBLIC
    cxx_nullptr
    cxx_override
)
target_link_libraries(coverage_test Qt5::Test CutelystQt5::Core Qt5::Network CutelystQt5::Utils::Validator)

macro(CUTELYST_TEMPLATES_UNIT_TESTS)
  foreach(_testname ${ARGN})
    add_executable(${_testname}_exec
                  ${_testname}.cpp
    )
    add_test(${_testname} ${_testname}_exec)
    target_compile_features(${_testname}_exec
      PRIVATE
        cxx_auto_type
      PUBLIC
        cxx_nullptr
        cxx_override
    )
    target_link_libraries(${_testname}_exec CutelystQt5::Core coverage_test)

    set_property(GLOBAL APPEND PROPERTY TEST_COVERAGE "${CMAKE_CURRENT_BINARY_DIR}/${_testname}_exec" )
  endforeach(_testname)
endmacro(CUTELYST_TEMPLATES_UNIT_TESTS)

cutelyst_templates_unit_tests(
    testheaders
    testcontext
    testrequest
    testresponse
    testdispatcherpath
    testdispatcherchained
    testvalidator
)

