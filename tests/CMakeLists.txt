project(cutelyst_tests VERSION 3)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Network Sql Test REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} 5.6.0 COMPONENTS Core Network Sql Test REQUIRED)

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
target_link_libraries(coverage_test Qt${QT_VERSION_MAJOR}::Test Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network)

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
    target_link_libraries(${_testname}_exec ${_link1} ${_link2} ${_link3} Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Core coverage_test)
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
    testactionrest
    testactionrenderview
)

if (${QT_VERSION} VERSION_LESS "6.1.0")
    cute_test(testcookie "" "" "")
endif (${QT_VERSION} VERSION_LESS "6.1.0")

cute_test(testvalidator Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Utils::Validator "" "")

cute_test(testauthentication Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Authentication Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Session "")
cute_test(testactionroleacl Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Authentication Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Session "")

cute_test(testpbkdf2 Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Authentication "" "")
cute_test(testpagination Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Utils::Pagination "" "")
cute_test(testviewjson Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::View::JSON "" "")
cute_test(teststatusmessage Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::StatusMessage Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Session "")
if (PLUGIN_MEMCACHED)
    cute_test(testmemcached Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Memcached "" "")
endif (PLUGIN_MEMCACHED)
cute_test(testlangselect Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Utils::LangSelect Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Session Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::StaticSimple)
cute_test(testlangselectmanual Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Utils::LangSelect Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::Session "")
if (PLUGIN_CSRFPROTECTION)
    cute_test(testcsrfprotection Cutelyst${PROJECT_VERSION_MAJOR}Qt${QT_VERSION_MAJOR}::CSRFProtection "" "")
endif(PLUGIN_CSRFPROTECTION)
