set(plugin_validator_SRC
    validator.cpp
    validator_p.h
    validatorrule.cpp
    validatorrule_p.h
    validatoraccepted.cpp
    validatoraccepted_p.h
    validatorafter.cpp
    validatorafter_p.h
    validatoralpha.cpp
    validatoralpha_p.h
    validatoralphadash.cpp
    validatoralphadash_p.h
    validatoralphanum.cpp
    validatoralphanum_p.h
    validatorbefore.cpp
    validatorbefore_p.h
    validatorbetween.cpp
    validatorbetween_p.h
    validatorboolean.cpp
    validatorboolean_p.h
    validatorconfirmed.cpp
    validatorconfirmed_p.h
    validatordate.cpp
    validatordate_p.h
    validatordatetime.cpp
    validatordatetime_p.h
    validatordifferent.cpp
    validatordifferent_p.h
    validatordigits.cpp
    validatordigits_p.h
    validatordigitsbetween.cpp
    validatordigitsbetween_p.h
    validatoremail.cpp
    validatoremail_p.h
    validatorfilesize.cpp
    validatorfilesize_p.h
    validatorfilled.cpp
    validatorfilled_p.h
    validatorin.cpp
    validatorin_p.h
    validatorinteger.cpp
    validatorinteger_p.h
    validatorip.cpp
    validatorip_p.h
    validatorjson.cpp
    validatorjson_p.h
    validatormax.cpp
    validatormax_p.h
    validatormin.cpp
    validatormin_p.h
    validatornotin.cpp
    validatornotin_p.h
    validatornumeric.cpp
    validatornumeric_p.h
    validatorpresent.cpp
    validatorpresent_p.h
    validatorregularexpression.cpp
    validatorregularexpression_p.h
    validatorrequired.cpp
    validatorrequired_p.h
    validatorrequiredif.cpp
    validatorrequiredif_p.h
    validatorrequiredifstash.cpp
    validatorrequiredifstash_p.h
    validatorrequiredunless.cpp
    validatorrequiredunless_p.h
    validatorrequiredunlessstash.cpp
    validatorrequiredunlessstash_p.h
    validatorrequiredwith.cpp
    validatorrequiredwith_p.h
    validatorrequiredwithall.cpp
    validatorrequiredwithall_p.h
    validatorrequiredwithout.cpp
    validatorrequiredwithout_p.h
    validatorrequiredwithoutall.cpp
    validatorrequiredwithoutall_p.h
    validatorsame.cpp
    validatorsame_p.h
    validatorsize.cpp
    validatorsize_p.h
    validatortime.cpp
    validatortime_p.h
    validatorurl.cpp
    validatorurl_p.h
    validatorresult.cpp
    validatorresult_p.h
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
    validatorfilesize.h
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
    validatorrequiredifstash.h
    validatorrequiredunless.h
    validatorrequiredunlessstash.h
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
    Qt5::Network
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
