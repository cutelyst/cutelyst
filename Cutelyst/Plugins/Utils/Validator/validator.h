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
#include <QObject>

namespace Cutelyst {

class ValidatorPrivate;
class Context;
class ValidatorRule;

/*!
 * \brief Validation processor for input data
 *
 * Validator can validate input data from the Context or a ParamsMultiMap using validation rules
 * implemented as classes derived from ValidatorRule. As long as setStopOnFirstError() is not set to \c true,
 * all validations will be performed untill the end. Validations will be performed in the order they were added
 * via addValidator(). Any field can have any amount of validators.
 *
 * The validators are designed to check common input data together with this processor, but might be used
 * for other purposes and standalone, too.
 *
 * Any validator requires at least to have the name of the field set that should be validated. Some validators have
 * additional mandatory parameters that have to be set. The ValidatorSame for example has a mandatory parameter to set
 * the name of the other field to compare the values.
 *
 * The main validator provides some comfort functions. One is the ability to set a label dictionary for generic error messages.
 * Every ValidatorRule will return a generic error message if no custom error message has been set. To use a label that is more
 * appropriate for displaying the field name and that should at best be the same as the label used in the HTML part, you could
 * set the label name for every validator. But if you are using more than one validator on a field, it might be easier to use
 * setLabelDictionary() to set a dictionary that automatically sets the label for every field and validator.
 *
 * The other comfort function can be used via setTemplate(). If you set the name of a template to the main Validator, it will
 * add all error information as well as the not sensible input data and the template name to the \link Context::stash() stash \endlink.
 *
 * \par Usage example
 *
 * As written before, validators can be used standalone, but that is not their intention - they are designed to validate input
 * data of form fields and API requests and so on. So they work best together with this main Validator and directly on the
 * Context \link Context::stash() stash \endlink.
 *
 * Most validators will succeed if the input field is empty. You should use them together with on of the required validators
 * if the input field is required. This approach is more flexible that having a simple swith in any validator. There are
 * different validators to require a field that make it possible to have more complex requirements. You can find information
 * about the behavior on empty input fields in the documenation of every validator. You can find some more general information
 * at ValidatorRule and for sure in the documentation for every single validator. Information about writing your own
 * validators that work with this concept can be found at ValidatorRule.
 *
 * \code{.cpp}
 * #include <Cutelyst/Plugins/Utils/Validator> // includes the main validator
 * #include <Cutelyst/Plugins/Utils/Validators> // includes all validator rules
 *
 * void MyController::myform(Context *c)
 * {
 *      if (c->req()->isPost()) {
 *
 *          Validator v(c);
 *
 *          // lets set a template to the validator to automaticall fill the stash with validation error information
 *          // this is optional, if we do not set it, the stash will not be filled/changed. you can request
 *          // the error data via errorStrings() and errorFields() and fill the stash by yourself
 *          v.setTemplate(QStringLiteral("myform.html");
 *
 *          // we will use a dictionary for the most of our validators to generate generic error messages
 *          // with appropriate field labels, the labels could also be set per validator, but when using
 *          // multiple validators per field it is easier to let the main Validator set them for all validators
 *          // used on the specific field
 *          v.setLabelDictionary({
 *                              {QStringLiteral("username"), tr("Username")},
 *                              {QStringLiteral("email"), tr("Email")}
 *                          });
 *
 *          // now lets add some nice validators
 *
 *          // this one will require the username to be present and not empty
 *          v.addValidator(new ValidatorRequired(QStringLiteral("username")));
 *
 *          // this one will require the username, if present (it has to be, see above),
 *          // to have a length between 3 and 255 characters
 *          v.addValidator(new ValidatorBetween(QStringLiteral("username"), QMetaType::QString, 3, 255));
 *
 *          // username can be long, but we dont want have anything else than alpha-numeric characters, dashes
 *          // and underscores in it - it should be a username, not a treaty
 *          v.addValidator(new ValidatorAlphaDash(QStringLiteral("username")));
 *
 *          // we also require an email address
 *          v.addValidator(new ValidatorRequired(QStringLiteral("email")));
 *
 *          // and damn sure, the email address should be valid, at least it should look like valid
 *          v.addValidator(new ValidatorEmail(QStringLiteral("email")));
 *
 *          // seems like we are building a registration form, so lets require a password
 *          v.addValidator(new ValidatorRequired(QStringLiteral("password")));
 *
 *          // the password should have a niminum length of 10 characters
 *          v.addValidator(new ValidatorMin(QStringLiteral("password"), QMetaType::QString, 10));
 *
 *          // and as true over the top usability experts we will have a password confirmation field (named password_confirmation)
 *          // but we don't like the generic error message and want to define a custom one
 *          v.addValidator(new ValidatorConfirmed(QStringLiteral("password"), QString(), tr("Please enter the same password again in the confirmation field.")));
 *
 *          // ok, now we have all our validators in place - let the games begin
 *          if (v.validate()) {
 *              // ok everything is valid, we can now process the input data and advance to the next
 *              // step for example
 *              c->response()->redirect(uriFor("nextstep"));
 *
 *              // but what happens if the input data was not valid?
 *              // because we set a template to the main Validator, it will automatically fill the stash
 *              // with error information so that our user can enter them correclty now
 *          }
 *
 *      } else {
 *          c->setStash({QStringLiteral("template), QStringLiteral("myform.html")});
 *      }
 *
 * }
 * \endcode
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT Validator : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief Constructs a new validator with given \a params and \a parent.
     */
    Validator(const ParamsMultiMap &params, QObject *parent = nullptr);

    /*!
     * \brief Constructs a new validator with given Context \a c and \a parent.
     *
     * The Context \a c will be used to extract the request parameters. If you set a template with setTemplate(),
     * the context's stash will be filled with error information if validation fails.
     */
    Validator(Context *c, QObject *parent = nullptr);

    /*!
     * \brief Desconstructs the validator.
     */
    ~Validator();

    /*!
     * \brief Set to \c true to stop the validation process on the first error.
     *
     * If this is set to \c true, the validation process will stop on the first validation
     * error and will not perform the following errors. The default value is \c false.
     *
     * \sa stopOnFirstError()
     */
    void setStopOnFirstError(bool stopOnFirstError);

    /*!
     * \brief Returns \c true if the validation process should stop on the first error.
     *
     * By default this will return \c false and the main Validator will process all added
     * validators.
     */
    bool stopOnFirstError() const;

    /*!
     * \brief Clears all internal data.
     *
     * Will clear the parameters and the used validators. ValidatorRule objects that are children of this
     * Validator will be destroyed.
     */
    void clear();

    /*!
     * \brief Starts the validation process and retruns \c true on success.
     *
     * The main Validator will start the validation of any added validator in the order they have been
     * added via the addValidator() function. If one of the validations fails, it will return \c false.
     */
    bool validate();

    /*!
     * \brief Adds a new validator to the list of validators.
     *
     * Adds a new ValidatorRule to the list of validators an sets this Validator as the parent of
     * the added rule if it has no parent. So the rules that were orphanes before will now have a
     * parent and will get destroyed on the parent's destruction. Horrible, isn't it? Rules that
     * were no orphanes when adding them to the validator should be destroyed by their parents to
     * free resources and make the club of rome happy.
     *
     * The Validator will process the single rules in the order they have been added.
     */
    void addValidator(ValidatorRule *v);

    /*!
     * \brief Returns a list of all error strings.
     *
     * If validation fails, this will return a list of error messages from every failed validator.
     */
    QVariantList errorStrings() const;

    /*!
     * \brief Returns a list of all fields with errors.
     *
     * If validation fails, this will return a list of field names where validation failed.
     */
    QVariantList errorFields() const;

    /*!
     * \brief Sets a new label dictionary.
     *
     * Sets a new dictionary that translates between field names and field labels. The entries are used to create generic error messages.
     * Every validator has a \link ValidatorRule::label label \endlink property that can be used to set the visible field label for generic error
     * messages. If there is no label and no custom error message set on a validator, a generic error message will be returned on failed validation
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
     * Adds \a labelDict to the internal label dictionary.
     *
     * \sa setLabelDictionary(), addLabel()
     */
    void addLabelDictionary(const QHash<QString,QString> &labelDict);

    /*!
     * \brief Adds a single entry to the label dictionary.
     *
     * Adds the \a label for the \a field to the internal label dictionary.
     *
     * \sa setLabelDictionary(), addLabelDictionary()
     */
    void addLabel(const QString &field, const QString &label);

    /*!
     * \brief Sets a template to process if validation fails.
     *
     * If you set a template and used a Context to construct the Validator, the Validator will add error information and input data
     * to the \link Context::stash() stash \endlink as well as the template name, if validation fails. If validation succeedes, Validator will
     * not modify the stash. This can be used to directly return error messages to the user and prefill the form with data entered by the user before.
     *
     * Validator will add all input fields with their names back to the stash, except those that contain the word \a password. Additionally it will set
     * the template to the \a template stash entry and will add two more entries to the stash: \a validationErrorStrings contains a list of all validation error messages
     * and \a validationErrorFields will contain a list of field names that have validation errors.
     *
     * \par Example
     *
     * \code{.cpp}
     * void MyController::MyForm(Context *c)
     * {
     *      if (c->req()->isPost()) {
     *          Validator v(c);
     *          v.setTemplate("myform.html");
     *          v.addValidator(new ValidatorRequired("name"));
     *          v.addValidator(new ValiadtorRequired("email"));
     *          v.addValidator(new ValidatorEmail("email"));
     *          v.addValidator(new ValidatorRequired("password"));
     *          v.addValidator(new ValidatorConfirmed("password"));
     *
     *          if (v.validate()) {
     *              // do something useful with the input data
     *              c->response()->redirect(uriFor("nextstep"));
     *          }
     *      } else {
     *          c->setStash("template", "myform.html");
     *      }
     * }
     * \endcode
     *
     * Lets now assume the user enters the following values:
     * \li \c username = detlef
     * \li \c email = detlef@irgendwo
     * \li \c password = schalke04
     * \li \c password_confirmation = schalke05
     *
     * The validation will fail, because the email address is not valid and the password confirmation does not match the password. The stash will contain the
     * following values:
     * \li \c username: "detlef"
     * \li \c email: "detlef@irgendwo"
     * \li \c validationErrorStrings: ["The email address in the “email” field is not valid.", "The content of the “password” field has not been confirmed."]
     * \li \c validationErrorFields: ["email", "password"]
     * \li \c template: "myform.html"
     *
     * The sensible data of the password fields is not part of the stash, but the other values can be used to prefill the form fields for the next attempt of
     * our little Schalke fan and can give him some hints what was wrong.
     */
    void setTemplate(const QString &tmpl);

protected:
    const QScopedPointer<ValidatorPrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(Validator)
    Q_DISABLE_COPY(Validator)
};

}

#endif //CUTELYSTVALIDATOR_H
