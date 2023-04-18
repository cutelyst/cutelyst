/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATOR_H
#define CUTELYSTVALIDATOR_H

#include "validatorresult.h"

#include <Cutelyst/ParamsMultiMap>
#include <Cutelyst/cutelyst_global.h>

#include <QScopedPointer>

namespace Cutelyst {

/*!
 * \defgroup plugins-utils-validator Validator
 * \brief Provides an API to validate input values.
 *
 * The %Validator plugin provides an API to validate input values that are send by a user to the application,
 * typically in the request body if it is a POST request or the URL query if it is a GET request. The plugin
 * already provides \link plugins-utils-validator-rules validator rules\endlink for common tasks and input
 * types but can be extended by deriving a new rule class from ValidatorRule. %Validator rules are not meant
 * to be used on their own but as part of the Validator class that processes the input data. However some
 * validator rules export their validation logic in a static member function that can be used without Validator
 * directly on a value. See the documentation of Validator to learn more about how to use it.
 *
 * <h3>Logging</h3>
 * Information is logged to the \c cutelyst.utils.validator logging category. Failed validation will only be logged
 * if debug output is enabled. Other errors like failed parsing and missing validation data will be logged to
 * the warning category.
 *
 * <h3>Building and using</h3>
 * The plugin is linked to %Cutelyst Core API and the QtNetwork module. To use it in your application, link your
 * application to \a Cutelyst::Utils::Validator.
 *
 * <h4>Optional validators</h4>
 * ValidatorPwQuality relies on <a href="https://github.com/libpwquality/libpwquality">libpwquality</a> and will not
 * be included and build by default. Use either <code>-DPLUGIN_VALIDATOR_PWQUALITY:BOOL=ON</code> or
 * <code>-DBUILD_ALL:BOOL=ON</code> when configuring %Cutelyst for build with cmake.
 */

class ValidatorPrivate;
class Context;
class Application;
class ValidatorRule;

/*!
 * \ingroup plugins-utils-validator
 * \class Validator validator.h <Cutelyst/Plugins/Utils/Validator>
 * \brief Validation processor for input data
 *
 * %Validator can validate input data from the Context or a ParamsMultiMap using validation rules
 * implemented as classes derived from ValidatorRule. As long as the Validator::StopOnFirstError flag is not set,
 * all validations will be performed until the end. Validations will be performed in the order they were added
 * on construction or via addValidator(). Any field can have any amount of validators. The %Validator will
 * take ownership of the added \link plugins-utils-validator-rules validator rules\endlink and will delete them
 * on it's own destruction.
 *
 * Any validator requires at least the name of the \a field that should be validated. Some validators have
 * additional mandatory parameters that have to be set. The ValidatorSame for example has a mandatory parameter to set
 * the name of the other field to compare the values.
 *
 * If you are using \link ValidatorMessages custom translatable error messages\endlink, you have to set the translation
 * context on the %Validator on the constructor. The same translation context has than to be used with the custom strings
 * for the validators. See ValidatorMessages for more information about translation of custom messages.
 *
 * Setting the \link Validator::FillStashOnError FillStashOnError\endlink flag on the validate() function will add all
 * error information as well as the not sensible (not containing the string \c "password" in the field name) input
 * data to the \link Context::stash() stash\endlink if validation fails.
 *
 * If only parameters from the request body or the request URL query should be taken into account by the validator,
 * use the \link Validator::BodyParamsOnly BodyParamsOnly\endlink or \link Validator::QueryParamsOnly QueryParamsOnyly\endlink
 * flag on the validate() function. If both are set, \link Validator::BodyParamsOnly BodyParamsOnly\endlink takes precedence.
 * If nothing is set, all parameters to validate will be taken from both, body and query.
 *
 * <h3>Usage example</h3>
 *
 * In %Cutelyst 1.x \link plugins-utils-validator-rules validator rules\endlink were usable standalone without being part
 * of a %Validator object - even though they were never meant to be. This changed in %Cutelyst 2.0.0 so that only the constructor
 * and destructor of a ValiadtorRule are public anymore. However they now can be used more flexible by pointing them to other
 * input fields or stash keys to get validation data like compare values. Some validator rules like the ValidatorEmail export
 * their validation logic as static function so that it can be used standalone directly on a value without needing a Context.
 *
 * Most validators will succeed if the input field is empty. You should use them together with one of the \link ValidatorRequired
 * required validators\endlink if the input field is required. This approach is more flexible than having a simple switch in any
 * validator. There are different validators to require a field that make it possible to have more complex requirements. You can
 * find information about the behavior on empty input fields in the documenation of every validator rule. You can find some more
 * general information at ValidatorRule and for sure in the documentation for every single \link plugins-utils-validator-rules
 * validator rule\endlink. Information about writing your own validators that work with this concept can be found at ValidatorRule.
 *
 * %Validator will return a ValidatorResult after validation has been performed. The result can be used to check the validity of the
 * input data. It will contain error messages from every failed ValidatorRule and a list of field names for which validation failed
 * as well as the extracted values from the fields under validation. If there are no failed validations, the result will be valid,
 * what you can check via ValidatorResult::isValid() or directly with an if statement.
 *
 * \code{.cpp}
 * #include <Cutelyst/Plugins/Utils/Validator> // includes the main validator
 * #include <Cutelyst/Plugins/Utils/Validators> // includes all validator rules
 * #include <Cutelyst/Plugins/Utils/ValidatorResult> // includes the validator result
 *
 * void MyController::myform(Context *c)
 * {
 *      if (c->req()->isPost()) {
 *
 *          // create a new static Validator with a set of rules and a translation context
 *          static Validator v({
 *              // this one will require the username to be present and not empty
 *              new ValidatorRequired(QStringLiteral("username")),
 *
 *              // this one will require the username, if present (it has to be, see above),
 *              // to have a length between 3 and 255 characters
 *              new ValidatorBetween(QStringLiteral("username"), QMetaType::QString, 3, 255),
 *
 *              // username can be long, but we dont want have anything else than ASCII alpha-numeric characters,
 *              // dashes and underscores in it
 *              new ValidatorAlphaDash(QStringLiteral("username"), true),
 *
 *              // we also require an email address
 *              new ValidatorRequired(QStringLiteral("email")),
 *
 *              // and damn sure, the email address should be valid, at least it should look like valid
 *              // we are using a custom validation error message without a label
 *              new ValidatorEmail(QStringLiteral("email"),
 *                                 ValidatorEmail::Valid, // really strict validation
 *                                 false, // we will not perform a DNS check
 *                                 ValidatorMessages(nullptr, QT_TRANSLATE_NOOP("MyController", "The email address in the Email field is not valid."))
 *                                ),
 *
 *              // seems like we are building a registration form, so lets require a password
 *              new ValidatorRequired(QStringLiteral("password")),
 *
 *              // the password should have a niminum length of 10 characters
 *              new ValidatorMin(QStringLiteral("password"), QMetaType::QString, 10),
 *
 *              // the user should confirm the password in another field
 *              // and here we are using a custom error message
 *              new ValidatorConfirmed(QStringLiteral("password"),
 *                                     ValidatorMessages(QT_TRANSLATE_NOOP("MyController", "Password"),
 *                                                       QT_TRANSLATE_NOOP("MyController", "Please enter the same password again in the confirmation field."))
 *                                    );
 *          }, QLatin1String("MyController"));
 *
 *          // ok, now we have all our validators in place - let the games begin
 *          // we will set the FillStashOnError flag to automatically fill the context stash with error data
 *          // and as you can see, we can directly use the ValidatorResult in an if statement, because of it's
 *          // bool operator in this situation, because we are filling the stash directly in the Valiator
 *          if (v.validate(c, FillStashOnError)) {
 *              // ok everything is valid, we can now process the input data and advance to the next
 *              // step for example
 *              c->response()->redirect(uriFor("nextstep"));
 *
 *              // but what happens if the input data was not valid?
 *              // because we set FillStashOnError, the Validator will automatically fill the stash
 *              // with error information so that our user can enter them correclty now
 *          }
 *
 *      }
 *
 *      c->setStash({QStringLiteral("template), QStringLiteral("myform.html")});
 * }
 * \endcode
 *
 * <h3>Automatically filling the stash</h3>
 *
 * If you set the \link Validator::FillStashOnError FillStashOnError\endlink flag on the validate() function,
 * the %Validator will automatically fill the \link Context::stash() stash \endlink of the Context with error
 * information and field values that are not sensible (field names that do not contain <code>"password"</code>).
 *
 * Beside the field values added with their field names, %Validator will add two more entries to the stash:
 * \li \a validationErrorStrings - a QStringList containing a list of all validation error messages, returned by ValidatorResult::errorStrings()
 * \li \a validationErrors - a QHash containing a dictionary with field names and their error strings, returned by ValidatorResult::errors()
 *
 * Let's assume that a user enters the following values into the form fields from the example above:
 * \li \c username = detlef
 * \li \c email = detlef\@irgendwo
 * \li \c password = schalke04
 * \li \c password_confirmation = schalke05
 *
 * The validation will fail, because the email address is not completely valid (it uses a TLD as domain, what is allowed according to RFC5321, but
 * we set the ValidatorEmail::Valid category as threshold for the validation, thas does not allow TLDs as domain part) and the password confirmation
 * does not match the password.
 *
 * %Validator will add the following entries to the \link Context::stash() stash \endlink:
 * \li \c username: "detlef"
 * \li \c email: "detlef@irgendwo"
 * \li \c validationErrorStrings: ["The email address in the Email field is not valid.", "Please enter the same password again in the confirmation field."]
 * \li \c validationErrors: ["email":["The email address in the Email field is not valid."], "password":[""Please enter the same password again in the confirmation field.""]]
 *
 * The sensible data of the password fields is not part of the stash, but the other values can be used to prefill the form fields for the next attempt of
 * our little Schalke fan and can give him some hints what was wrong.
 *
 * <h3>Usage with Grantlee</h3>
 *
 * The following example shows possible usage of the error data with \link GrantleeView Grantlee \endlink and the Bootstrap3 framework.
 *
 * \code{.html}
 * {% if validationErrorStrings.count %}
   <div class="alert alert-warning" role="alert">
       <h4 class="alert-heading">Errors in input data</h4>
       <p>
       <ul>
           {% for errorString in validationErrorStrings %}
           <li>{{ errorString }}</li>
           {% endfor %}
       </ul>
       </p>
   </div>
   {% endif %}

   <form>

       <div class="form-group{% if validationErrors.email.count %} has-warning{% endif %}">
           <label for="email">Email</label>
           <input type="email" id="email" name="email" maxlength="255" class="form-control{% if validationErrors.email.count %} form-control-warning{% endif %}" placeholder="Email" aria-describedby="emailHelpBlock" required value="{{ email }}">
           <small id="emailHelpBlock" class="form-text text-muted">The email address will be used to send notifications and to restore lost passwords. Maximum length: 255</small>
       </div>

   </form>
 * \endcode
 *
 * <h3>Translations</h3>
 *
 * Use Validator::loadTranslations(this) in your reimplementation of Application::init() if you are using the %Validator
 * plugin and want to use translated generic messages.
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT Validator
{
public:
    /*!
     * \brief Flags that change the behavior of the Validator.
     */
    enum ValidatorFlag {
        NoSpecialBehavior = 0, /**< No special behavior, the default. */
        StopOnFirstError  = 1, /**< Will stop the validation process on the first failed validation. */
        FillStashOnError  = 2, /**< Will fill the context's stash with error information. */
        NoTrimming        = 4, /**< Will set \link ValidatorRule::trimBefore() trimBefore()\endlink to \c false on every validator. (default behavior is \c true) */
        BodyParamsOnly    = 8, /**< Will only check for parameters that are send in the \link Request::bodyParameters() request body\endlink. (since Cutelyst 2.0.0) */
        QueryParamsOnly   = 16 /**< Will only check for parameters that are part of the \link Request::queryParameters() request URL query\endlink. (since Cutelyst 2.0.0) */
    };
    Q_DECLARE_FLAGS(ValidatorFlags, ValidatorFlag)

    /*!
     * \brief Constructs a new %Validator.
     */
    explicit Validator(QLatin1String translationContext = QLatin1String());

#ifdef Q_COMPILER_INITIALIZER_LISTS
    /*!
     * \brief Constructs a new %Validator using the defined \a validators.
     * \param validators List of validators that should be performed on the input fields. The %Validator will take ownerhip
     * of them and will destroy them on it's own destruction.
     *
     * This constructor is only available if the compiler supports C++11 std::initializer_list.
     */
    explicit Validator(std::initializer_list<ValidatorRule *> validators, QLatin1String translationContext = QLatin1String());
#endif

    /*!
     * \brief Desconstructs the Validator and all added ValidatorRule objects.
     */
    ~Validator();

    /*!
     * \brief Clears all internal data.
     *
     * Will clear the parameters and the used validators. ValidatorRule objects that have been added
     * to the Validator will get destroyed.
     */
    void clear();

    /*!
     * \brief Starts the validation process on Context \a c and returns a ValidatorResult.
     *
     * Requests the input parameters from Context \a c and processes any validator added through
     * the constructor or via addValidator() (unless Validator::StopOnFirstError is set). Returns a
     * ValidatorResult that contains information about validation errors, if any. The result can be checked
     * directly in \c if statements.
     *
     * If Validator::FillStashOnError is set, it will fill the stash of Context \c with error data and not
     * sensible input values.
     */
    ValidatorResult validate(Context *c, ValidatorFlags flags = NoSpecialBehavior) const;

    /*!
     * \brief Starts the validation process on the \a parameters and returns \c true on success.
     *
     * Processes any validator added through the constructor or via addValidator() (unless Validator::StopOnFirstError is set).
     * Returns a ValidatorResult that contains information about validation errors, if any. The result can be checked
     * directly in \c if statements.
     *
     * If Validator::FillStashOnError is set, it will fill the stash of Context \c with error data and not
     * sensible input values.
     */
    ValidatorResult validate(Context *c, const ParamsMultiMap &parameters, ValidatorFlags flags = NoSpecialBehavior) const;

    /*!
     * \brief Adds a new validator to the list of validators.
     *
     * Adds a new ValidatorRule to the list of validators. On destruction of the Validator,
     * all added rules will get destroyed, too.
     */
    void addValidator(ValidatorRule *v);

    /*!
     * \brief Loads the translations for the plugin.
     */
    static void loadTranslations(Application *app);

protected:
    const QScopedPointer<ValidatorPrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(Validator)
    Q_DISABLE_COPY(Validator)
};

} // namespace Cutelyst

Q_DECLARE_OPERATORS_FOR_FLAGS(Cutelyst::Validator::ValidatorFlags)

#endif // CUTELYSTVALIDATOR_H
