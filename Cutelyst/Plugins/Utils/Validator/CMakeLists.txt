set(plugin_validator_SRC
    validator.cpp
    validator_p.h
    validator.h
    validatorrule.cpp
    validatorrule_p.h
    validatorrule.h
    validatoraccepted.cpp
    validatoraccepted_p.h
    validatoraccepted.h
    validatorafter.cpp
    validatorafter_p.h
    validatorafter.h
    validatoralpha.cpp
    validatoralpha_p.h
    validatoralpha.h
    validatoralphadash.cpp
    validatoralphadash_p.h
    validatoralphadash.h
    validatoralphanum.cpp
    validatoralphanum_p.h
    validatoralphanum.h
    validatorbefore.cpp
    validatorbefore_p.h
    validatorbefore.h
    validatorbetween.cpp
    validatorbetween_p.h
    validatorbetween.h
    validatorboolean.cpp
    validatorboolean_p.h
    validatorboolean.h
    validatorconfirmed.cpp
    validatorconfirmed_p.h
    validatorconfirmed.h
    validatordate.cpp
    validatordate_p.h
    validatordate.h
    validatordatetime.cpp
    validatordatetime_p.h
    validatordatetime.h
    validatordifferent.cpp
    validatordifferent_p.h
    validatordifferent.h
    validatordigits.cpp
    validatordigits_p.h
    validatordigits.h
    validatordigitsbetween.cpp
    validatordigitsbetween_p.h
    validatordigitsbetween.h
    validatoremail.cpp
    validatoremail_p.h
    validatoremail.h
    validatorfilled.cpp
    validatorfilled_p.h
    validatorfilled.h
    validatorin.cpp
    validatorin_p.h
    validatorin.h
    validatorinteger.cpp
    validatorinteger_p.h
    validatorinteger.h
    validatorip.cpp
    validatorip_p.h
    validatorip.h
    validatorjson.cpp
    validatorjson_p.h
    validatorjson.h
    validatormax.cpp
    validatormax_p.h
    validatormax.h
    validatormin.cpp
    validatormin_p.h
    validatormin.h
    validatornotin.cpp
    validatornotin_p.h
    validatornotin.h
    validatornumeric.cpp
    validatornumeric_p.h
    validatornumeric.h
    validatorpresent.cpp
    validatorpresent_p.h
    validatorpresent.h
    validatorregularexpression.cpp
    validatorregularexpression_p.h
    validatorregularexpression.h
    validatorrequired.cpp
    validatorrequired_p.h
    validatorrequired.h
    validatorrequiredif.cpp
    validatorrequiredif_p.h
    validatorrequiredif.h
    validatorrequiredunless.cpp
    validatorrequiredunless_p.h
    validatorrequiredunless.h
    validatorrequiredwith.cpp
    validatorrequiredwith_p.h
    validatorrequiredwith.h
    validatorrequiredwithall.cpp
    validatorrequiredwithall_p.h
    validatorrequiredwithall.h
    validatorrequiredwithout.cpp
    validatorrequiredwithout_p.h
    validatorrequiredwithout.h
    validatorrequiredwithoutall.cpp
    validatorrequiredwithoutall_p.h
    validatorrequiredwithoutall.h
    validatorsame.cpp
    validatorsame_p.h
    validatorsame.h
    validatorsize.cpp
    validatorsize_p.h
    validatorsize.h
    validatortime.cpp
    validatortime_p.h
    validatortime.h
    validatorurl.cpp
    validatorurl_p.h
    validatorurl.h
    validatorresult.cpp
    validatorresult_p.h
    validatorresult.h
) 


set(plugin_validator_HEADERS
    validator.h
    Validator
    validatorrule.h
    ValidatorRule
    validatoraccepted.h
    validatorafter.h
    validatoralpha.h
    validatoralphadash.h
    validatoralphanum.h
    validatorbefore.h
    validatorbetween.h
    validatorboolean.h
    validatorconfirmed.h
    validatordate.h
    validatordatetime.h
    validatordifferent.h
    validatordigits.h
    validatordigitsbetween.h
    validatoremail.h
    validatorfilled.h
    validatorin.h
    validatorinteger.h
    validatorip.h
    validatorjson.h
    validatormax.h
    validatormin.h
    validatornotin.h
    validatornumeric.h
    validatorpresent.h
    validatorregularexpression.h
    validatorrequired.h
    validatorrequiredif.h
    validatorrequiredunless.h
    validatorrequiredwith.h
    validatorrequiredwithall.h
    validatorrequiredwithout.h
    validatorrequiredwithoutall.h
    validatorsame.h
    validatorsize.h
    validatortime.h
    validatorurl.h
    Validators
    validatorresult.h
    ValidatorResult
)

add_library(Cutelyst2Qt5UtilsValidator SHARED
    ${plugin_validator_SRC}
    ${plugin_validator_HEADERS}
)
add_library(Cutelyst2Qt5::Utils::Validator ALIAS Cutelyst2Qt5UtilsValidator)

set_target_properties(Cutelyst2Qt5UtilsValidator PROPERTIES
    EXPORT_NAME Utils::Validator
    VERSION ${PROJECT_VERSION}
    SOVERSION ${CUTELYST_API_LEVEL}
)

target_link_libraries(Cutelyst2Qt5UtilsValidator
    PRIVATE Cutelyst2Qt5::Core
)

set_property(TARGET Cutelyst2Qt5UtilsValidator PROPERTY PUBLIC_HEADER ${plugin_validator_HEADERS})
install(TARGETS Cutelyst2Qt5UtilsValidator
    EXPORT CutelystTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION include/cutelyst2-qt5/Cutelyst/Plugins/Utils
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CutelystQt5UtilsValidator.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5UtilsValidator.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Cutelyst2Qt5UtilsValidator.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
