/*
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef CUTELYSTVALIDATOR_H
#define CUTELYSTVALIDATOR_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/ParamsMultiMap>
#include <QScopedPointer>
#include "validatorresult.h"

namespace Cutelyst {

class ValidatorPrivate;
class Context;
class ValidatorRule;

/*!
 * \brief Validation processor for input data
 *
 * Validator can validate input data from the Context or a ParamsMultiMap using validation rules
 * implemented as classes derived from ValidatorRule. As long as the Validator::StopOnFirstError flasg is not set,
 * all validations will be performed until the end. Validations will be performed in the order they were added
 * on construction or via addValidator(). Any field can have any amount of validators.
 *
 * Any validator requires at least the name of the \a field that should be validated. Some validators have
 * additional mandatory parameters that have to be set. The ValidatorSame for example has a mandatory parameter to set
 * the name of the other field to compare the values.
 *
 * The main Validator provides some comfort functions. One is the ability to set a label dictionary for generic error messages.
 * Every ValidatorRule will return a generic error message if no custom error message has been set. To use a label that is more
 * appropriate for displaying the field name and that should at best be the same as the label used in the HTML part, you could
 * set the label name on every validator. But if you are using more than one validator on a field, it might be easier to use
 * the constructor that takes the label dictionary as list or to use setLabelDictionary() to set a dictionary that automatically sets
 * the label for every field and validator.
 *
 * The other comfort function can be used via the Validator::FillStashOnError flag. If you set the flag on the validate() function that takes
 * the Context pointer as parameter, it will add all error information as well as the not sensible input data to the \link Context::stash() stash \endlink
 * using the fillStash() function.
 *
 * \par Usage example
 *
 * Validators can be used standalone, but that is not their intention - they are designed to validate input
 * data of form fields and API requests and so on. So they work best together with this main Validator and directly on the
 * Context \link Context::stash() stash \endlink.
 *
 * Most validators will succeed if the input field is empty. You should use them together with one of the required validators
 * if the input field is required. This approach is more flexible than having a simple switch in any validator. There are
 * different validators to require a field that make it possible to have more complex requirements. You can find information
 * about the behavior on empty input fields in the documenation of every validator. You can find some more general information
 * at ValidatorRule and for sure in the documentation for every single validator. Information about writing your own
 * validators that work with this concept can be found at ValidatorRule.
 *
 * Validator will retrun a ValidatorResult after validation has been performed. The result can be used to check the validity of the
 * input data. It will contain error messages from every failed ValidatorRule and a list of field names for which validation failed.
 * If there are no failed validation, the result will be valid, what you can check via ValidatorResult::isValid() or directly with
 * an if statement.
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
 *          // create a new static Validator with a set of rules and a label dictionary
 *          static Validator v({
 *              // this one will require the username to be present and not empty
 *              new ValidatorRequired(QStringLiteral("username")),
 *
 *              // this one will require the username, if present (it has to be, see above),
 *              // to have a length between 3 and 255 characters
 *              new ValidatorBetween(QStringLiteral("username"), QMetaType::QString, 3, 255),
 *
 *              // username can be long, but we dont want have anything else than alpha-numeric characters, dashes
 *              // and underscores in it
 *              new ValidatorAlphaDash(QStringLiteral("username")),
 *
 *              // we also require an email address
 *              new ValidatorRequired(QStringLiteral("email")),
 *
 *              // and damn sure, the email address should be valid, at least it should look like valid
 *              new ValidatorEmail(QStringLiteral("email")),
 *
 *              // seems like we are building a registration form, so lets require a password
 *              new ValidatorRequired(QStringLiteral("password")),
 *
 *              // the password should have a niminum length of 10 characters
 *              new ValidatorMin(QStringLiteral("password"), QMetaType::QString, 10),
 *
 *              // the user should confirm the password in another field
 *              // and here we are using a custom error message
 *              new ValidatorConfirmed(QStringLiteral("password"), QString(), tr("Please enter the same password again in the confirmation field."))
 *          }, {
 *              // we will use a dictionary for our validators to generate generic error messages with
 *              // appropriate field labels, the labels could also be set per validator, but when using
 *              // multiple validators per field it is easier to let the main Validator set them for all
 *              // validators used on the specific field
 *              {QStringLiteral("username"), QStringLiteral("Username")},
 *              {QStringLiteral("email"), QStringLiteral("Email")},
 *              {QStringLiteral("password"), QStringLiteral("Password")}
 *          });
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
 * \par Automatically filling the stash
 *
 * If you set the Validator::FillStashOnError on the validate() function that takes a Context pointer,
 * the Validator will automatically fill the \link Context::stash() stash \endlink of the Context with error
 * information and field values that are not sensible (field names that do not contain \c password).
 *
 * Beside the field values added with their field names, Validator will add two more entries to the stash:
 * \li \a validationErrorStrings - a QStringList containing a list of all validation error messages, returned by ValidatorResult::errorStrings()
 * \li \a validationErrors - a QHash containing a dictionary with field names and their error strings, returned by ValidatorResult::errors()
 *
 * Let's assume that a user enters the following values into the form fields from the above example:
 * \li \c username = detlef
 * \li \c email = detlef\@irgendwo
 * \li \c password = schalke04
 * \li \c password_confirmation = schalke05
 *
 * The validation will fail, because the email address is not valid and the password confirmation does not match the password.
 * Validator will add the following entries to the \link Context::stash() stash \endlink:
 * \li \c username: "detlef"
 * \li \c email: "detlef@irgendwo"
 * \li \c validationErrorStrings: ["The email address in the Email field is not valid.", "Please enter the same password again in the confirmation field."]
 * \li \c validationErrors: ["email":["The email address in the Email field is not valid."], "password":[""Please enter the same password again in the confirmation field.""]]
 *
 * The sensible data of the password fields is not part of the stash, but the other values can be used to prefill the form fields for the next attempt of
 * our little Schalke fan and can give him some hints what was wrong.
 *
 * \par Usage with Grantlee
 *
 * The following example shows possible usage of the error data with \link GrantleeView Grantlee \endlink and the Bootstrap framework.
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
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT Validator
{
public:
    /*!
     * \brief Flags that change the behavior of the Validator.
     */
    enum ValidatorFlag {
        NoSpecialBehavior   = 0,    /**< No special behavior, the default. */
        StopOnFirstError    = 1,    /**< Will stop the validation process on the first failed validation. */
        FillStashOnError    = 2,    /**< Will fill the context's stash with error information. Will therefore only have an effect, if validate() has been started with a valid Context. */
        NoTrimming          = 4,    /**< Will set ValidatorRule::setTrimBefore() to \c false on every validator. (default behavior is \c true) */
    };
    Q_DECLARE_FLAGS(ValidatorFlags, ValidatorFlag)

    /*!
     * \brief Constructs a new Validator.
     */
    Validator();

#ifdef Q_COMPILER_INITIALIZER_LISTS
    /*!
     * \brief Constructs a new Validator using the defined \a validators.
     * \param validators    List of validators that should be performed on the input fields. Will get destroyed on Validator destruction.
     *
     * This constructor is only available if the compiler supports C++11 std::initializer_list.
     */
    explicit Validator(std::initializer_list<ValidatorRule*> validators);

    /*!
     * \brief Constructs a new Validator using the defined \a validators and \a labelDictionary.
     * \param validators        List of validators that should be performed on the input fields. Will get destroyed on Validator destruction.
     * \param labelDictionary   Dictionary translating the field names into human readable labels for generic error messages.
     *
     * This constructor is only available if the compiler supports C++11 std::initializer_list.
     */
    Validator(std::initializer_list<ValidatorRule*> validators, std::initializer_list<std::pair<QString,QString> > labelDictionary);
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
     * Validator::FillStashOnError will not have any effect if using this function.
     */
    ValidatorResult validate(const ParamsMultiMap &parameters, ValidatorFlags flags = NoSpecialBehavior) const;

    /*!
     * \brief Adds a new validator to the list of validators.
     *
     * Adds a new ValidatorRule to the list of validators. On destruction of the Validator,
     * all added rules will get destroyed, too.
     */
    void addValidator(ValidatorRule *v);

    /*!
     * \brief Sets a new label dictionary.
     *
     * Sets a new dictionary that translates between field names and field labels. The entries are used to create generic error messages.
     * Every validator can store field \link ValidatorRule::label() label \endlink that can be used to for generic error messages
     * If there is no label and no custom error message set on a validator, a generic error message will be returned on failed validation
     * that contains the field name. The field name might be looking ugly and uninformative to the user, so now the label comes into the game.
     *
     * You can set a label on each validator separately, but if you are using multiple validators on the same field, it might be easier to define
     * them here. The main validator will then set the label to every validator for that field that has no own label set.
     *
     * The key of the QHash is the field name, the value is the label.
     *
     * \sa addLabelDictionary(), addLabel()
     */
    void setLabelDictionary(const QHash<QString,QString> &labelDict);

    /*!
     * \brief Adds a dictionary to the label dictionary.
     *
     * Adds \a labelDict to the internal label dictionary. The key of the QHash is the field name, the value is the label.
     * See setLabelDictionary() for further information.
     *
     * \sa setLabelDictionary(), addLabel()
     */
    void addLabelDictionary(const QHash<QString,QString> &labelDict);

    /*!
     * \brief Adds a single entry to the label dictionary.
     *
     * Adds the \a label for the \a field to the internal label dictionary. See setLabelDictionary() for further information.
     *
     * \sa setLabelDictionary(), addLabelDictionary()
     */
    void addLabel(const QString &field, const QString &label);

protected:
    const QScopedPointer<ValidatorPrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(Validator)
    Q_DISABLE_COPY(Validator)
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Cutelyst::Validator::ValidatorFlags)

#endif //CUTELYSTVALIDATOR_H
