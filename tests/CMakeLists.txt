project(cutelyst_tests)

find_package(Qt5Test 5.6.0 REQUIRED)

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
target_link_libraries(coverage_test Qt5::Test CutelystQt5::Core Qt5::Network)

function(cute_test _testname _link1 _link2 _link3)
    add_executable(${_testname}_exec ${_testname}.cpp)
    add_test(NAME ${_testname} COMMAND ${_testname}_exec)
    target_compile_features(${_testname}_exec
      PRIVATE
        cxx_auto_type
      PUBLIC
        cxx_nullptr
        cxx_override
    )
    target_link_libraries(${_testname}_exec ${_link1} ${_link2} ${_link3} CutelystQt5::Core coverage_test)
endfunction()

macro(CUTELYST_TEMPLATES_UNIT_TESTS)
    foreach(_testname ${ARGN})
        cute_test(${_testname} "" "" "")
    endforeach(_testname)
endmacro(CUTELYST_TEMPLATES_UNIT_TESTS)

cutelyst_templates_unit_tests(
    testheaders
    testcontext
    testrequest
    testresponse
    testdispatcherpath
    testdispatcherchained
    testactionrest
    testactionrenderview
)

cute_test(testvalidator CutelystQt5::Utils::Validator "" "")

if (NOT CMAKE_GENERATOR MATCHES "Visual Studio" AND NOT CMAKE_GENERATOR MATCHES "Ninja")
    cute_test(testauthentication CutelystQt5::Authentication CutelystQt5::Session "")
    cute_test(testactionroleacl CutelystQt5::Authentication CutelystQt5::Session "")
endif()

cute_test(testpbkdf2 CutelystQt5::Authentication "" "")
cute_test(testpagination CutelystQt5::Utils::Pagination "" "")
cute_test(testviewjson CutelystQt5::View::JSON "" "")
cute_test(teststatusmessage CutelystQt5::StatusMessage CutelystQt5::Session "")
