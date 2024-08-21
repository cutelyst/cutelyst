project(cutelyst_tests
    LANGUAGES CXX
    VERSION 4
)

find_package(QT NAMES Qt6 COMPONENTS Core Network Sql Test REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} 5.6.0 COMPONENTS Core Network Sql Test REQUIRED)

add_library(coverage_test STATIC coverageobject.cpp
  coverageobject.h
)
target_link_libraries(coverage_test Qt::Test Cutelyst::Core Qt::Network)

function(cute_test _testname _link1 _link2 _link3)
    add_executable(${_testname}_exec ${_testname}.cpp)
    add_test(NAME ${_testname} COMMAND ${_testname}_exec)
    target_link_libraries(${_testname}_exec ${_link1} ${_link2} ${_link3} Cutelyst::Core coverage_test)
endfunction()

macro(CUTELYST_TEMPLATES_UNIT_TESTS)
    foreach(_testname ${ARGN})
        cute_test(${_testname} "" "" "")
    endforeach(_testname)
endmacro(CUTELYST_TEMPLATES_UNIT_TESTS)

cutelyst_templates_unit_tests(
    testheaders
    testcontext
    testcontroller
    testrequest
    testresponse
    testdispatcherpath
    testdispatcherchained
    tst_dispatcher
)

cute_test(testvalidator Cutelyst::Utils::Validator "" "")

cute_test(testauthentication Cutelyst::Authentication Cutelyst::Session "")

if(CUTELYST_PLUGIN_LINK_TYPE STREQUAL "STATIC")
    cute_test(testactionroleacl Cutelyst::Authentication Cutelyst::Session CutelystRoleACL)
    cute_test(testactionrest CutelystActionREST "" "")
    cute_test(testactionrenderview CutelystActionRenderView "" "")
else()
    cute_test(testactionroleacl Cutelyst::Authentication Cutelyst::Session "")
    cute_test(testactionrest "" "" "")
    cute_test(testactionrenderview "" "" "")
endif()

cute_test(testpbkdf2 Cutelyst::Authentication "" "")
cute_test(testpagination Cutelyst::Utils::Pagination "" "")
cute_test(testviewjson Cutelyst::View::JSON "" "")
cute_test(teststatusmessage Cutelyst::StatusMessage Cutelyst::Session "")
if (PLUGIN_MEMCACHED)
    cute_test(testmemcached Cutelyst::Memcached "" "")
endif (PLUGIN_MEMCACHED)
cute_test(testlangselect Cutelyst::Utils::LangSelect Cutelyst::Session Cutelyst::StaticSimple)
cute_test(testlangselectmanual Cutelyst::Utils::LangSelect Cutelyst::Session "")
if (PLUGIN_CSRFPROTECTION)
    cute_test(testcsrfprotection Cutelyst::CSRFProtection "" "")
endif(PLUGIN_CSRFPROTECTION)
cute_test(testutils "" "" "")
if (PLUGIN_STATICCOMPRESSED)
    cute_test(teststaticcompressed Cutelyst::StaticCompressed "" "")
endif (PLUGIN_STATICCOMPRESSED)
cute_test(teststaticsimple Cutelyst::StaticSimple "" "")
cute_test(testserver Cutelyst::Server "" "")
