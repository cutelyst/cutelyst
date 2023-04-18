/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORRULE_H
#define CUTELYSTVALIDATORRULE_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/paramsmultimap.h>

#include <QLoggingCategory>
#include <QScopedPointer>
#include <QVariant>

Q_DECLARE_LOGGING_CATEGORY(C_VALIDATOR)

namespace Cutelyst {

/*!
 * \ingroup plugins-utils-validator
 * \defgroup plugins-utils-validator-rules Rules
 * \brief Classes providing rules to validate input data.
 *
 * All validator rule classes are derived from ValidatorRule and are meant to be used as part of
 * Validator. Read the documentation of Validator to learn more about how to use the Validator and
 * the rules. If you want to write your own validator rule, create a new class that is derived from
 * ValidatorRule and reimplement the validate function.
 *
 * There are three constructor arguments that are common to almos all validator rules: the name of the
 * field to validate (mandatory), a struct containing translatable ValidatorMessages (optional) and
 * a \link Context::stash() stash\endlink key name that contains a default value if the input field
 * is not available (optional available where it makes sense).
 *
 * Some validators export their validation logic in a <a href="#func-members">static member function</a>
 * so that it can be used without creating a Validator.
 */

class Context;

/*!
 * \ingroup plugins-utils-validator
 * \class ValidatorReturnType validatorrule.h <Cutelyst/Plugins/Utils/ValidatorRule>
 * \brief Contains the result of a single input parameter validation.
 *
 * For information about the possible values of \link ValidatorReturnType::value value\endlink and
 * \link ValidatorReturnType::extra extra\endlink see the documentation of the respective
 * \link plugins-utils-validator-rules validator rule\endlink.
 */
struct CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorReturnType {
    QString errorMessage; /**< Contains a human readable error message if validation failed that should provide information about the reason the validation failed. If QString::isNull() returns \c true for this, the validation has succeeded and isValid() will alsor return \c true. */
    QVariant value;       /**< Might contain the extracted and possibly converted value of the \link ValidatorRule::value() input parameter\endlink if validation succeeded. For information about the possible values see the documentation of the respective \link plugins-utils-validator-rules validator\endlink. */
    QVariant extra;       /**< Might contain extra data provided by the validator. For information about the possible values see the documentation of the respective \link plugins-utils-validator-rules validator\endlink. */

    /*!
     * \brief Returns \c true if validation succeeded.
     * \return \c true if \link ValidatorReturnType::errorMessage errorMessage\endlink is a \link QString::isNull() null string\endlink,
     * indicating that the validation has succeeded.
     */
    explicit operator bool() const
    {
        return errorMessage.isNull();
    }

    /*!
     * \brief Returns \c true if validation succeeded.
     * \return \c true if \link ValidatorReturnType::errorMessage errorMessage\endlink is a \link QString::isNull() null string\endlink,
     * indicating that the validation has succeeded.
     */
    bool isValid() const
    {
        return errorMessage.isNull();
    }
};

/*!
 * \ingroup plugins-utils-validator
 * \class ValidatorMessages validatorrule.h <Cutelyst/Plugins/Utils/ValidatorRule>
 * \brief Stores custom error messages and the input field label.
 *
 * This struct is used by ValidatorRule derived classes to store custom error messages
 * that are also translatable. To make the messages translatable, use QT_TRANSLATE_NOOP() so
 * that the message can be dynamically translated by the current \link Context::translate() Context \endlink.
 * If you want to omit a custom message, simply use a \c nullptr for it. For custom messages that are
 * not set, the ValidatorRule class will return a generic message that will also be translated if
 * %Cutelyst has a translation file for the \link Context::locale() current language in the context\endlink.
 *
 * The translation context used in the QT_TRANSLATE_NOOP() definition has to be the same that has been set
 * on the Validator contstructor. If you dont want to use translation for the messages and the label, simply
 * don't use QT_TRANSLATE_NOOP() when adding the string but simply use a C string literal and also leave the
 * translation context on the Validator constructor empty.
 *
 * <h3>Usage example</h3>
 * \code{.cpp}
 * void MyController::do_form(Context *c)
 * {
 *     static Validator v1({
 *                         new ValidatorAccepted(QStringLiteral("usage_terms"),
 *                                               // empty label and translatable validation error message
 *                                               ValidatorMessages(nullptr, QT_TRANSLATE_NOOP("MyController", "Please accept our terms of usage to finish your registration.")),
 *
 *                         new ValidatorEmail(QStringLiteral("email"), false, ValidatorEmail::Valid,
 *                                            // only the label will be translated and inserted into generic error messages of the validator rule
 *                                            ValidatorMessages(QT_TRANSLATE_NOOP("MyController", "Your Email Address"))
 *
 *                         new ValidatorDate(QStringLiteral("birthday"),
 *                                           // here we also use a translatable version of the date format
 *                                           QT_TRANSLATE_NOOP("MyControler", "yyyy-MM-dd"),
 *                                           // this one will use a translated label for generic parsing and validation data
 *                                           // error messages and a custom translatable error message for failed validations
 *                                           ValidatorMessages(QT_TRANSLATE_NOOP("MyController", "You day of birth"),
 *                                                             QT_TRANSLATE_NOOP("MyController", "Please enter a valid date of birth"))
 *
 *                         // this uses a default constructed ValidatorMessages struct where
 *                         // no custom messages have been set, so all error messages will come
 *                         // from the generic validator messages without a label
 *                         new ValidatorRequired(QStringLiteral("password"));
 *
 *                          // this is the context that will be used to obtain translations,
 *                          // it has to be the same one you use in your validator messages
 *                          // QT_TRANSLATE_NOOP()
 *                        }, QLatin1String("MyController"));
 *
 *
 *     // this validator does not specify a translation context and will therefore not translate error messages,
 *     // even if they were added with QT_TRANSLATE_NOOP()
 *     static Validator v2({
 *                         new ValidatorRequired(QStringLiteral("required_field"),
 *                                               ValidatorMessages("Required Field", "This field is required, please enter data."))
 *                        });
 * }
 * \endcode
 */
struct CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorMessages {
    /*!
     * \brief Constructs a default %ValidatorMessages object with all custom messages disabled.
     */
    ValidatorMessages() {}
    /*!
     * \brief Constructs a new %ValidatorMessages object with the given parameters.
     *
     * \param customLabel               User visible label for the input field. Should be the same as used on the frontend visible to the user. Will be used by generic error messages if set.
     * \param customValidationError     Custom error message if the validation fails.
     * \param customParsingError        Custom error message if the input value could not be parsed.
     * \param customValidationDataError Custom error message if validation data is missing or invalid.
     */
    ValidatorMessages(const char *customLabel, const char *customValidationError = nullptr, const char *customParsingError = nullptr, const char *customValidationDataError = nullptr)
        : label(customLabel)
        , validationError(customValidationError)
        , parsingError(customParsingError)
        , validationDataError(customValidationDataError)
    {
    }
    const char *label               = nullptr; /**< Field label used for generating generic error messages. */
    const char *validationError     = nullptr; /**< Custom validation error messages. */
    const char *parsingError        = nullptr; /**< Custom parsing error message. */
    const char *validationDataError = nullptr; /**< Custom validation data error message. */
};

class ValidatorRulePrivate;

/*!
 * \ingroup plugins-utils-validator
 * \class ValidatorRule validatorrule.h <Cutelyst/Plugins/Utils/ValidatorRule>
 * \brief Base class for all validator \link plugins-utils-validator-rules rules\endlink.
 *
 * This class can not be used on it's own, you have to create a derived class from it that implements your
 * validator logic. Or use one of the \link plugins-utils-validator-rules already existing derived validator rules\endlink.
 *
 * \par Writing a custom validator
 *
 * If you want to implement your own validator logic to use with Validator, you have to create a class that
 * derives from ValidatorRule. The simplest implementation only needs a constructor and a reimplementation
 * of the validate() function. But for more comfort and usability, you should also reimplement genericValidationError().
 * If your validator parses the input into a specific type to validate it and/or if you are using additional parameters,
 * you may also want to reimplement genericParsingError() and genericValidationDataError() to return more appropriate
 * generic error messages.
 *
 * The most important parameter for every validator rule is the name of the \a field to validate. So your own validator
 * should require that field in the constructor. For better error messages you should also add an optional paramter to set
 * custom ValidatorMessages if validation fails.
 *
 * In the validation logic implemented in the validate() function you have to return a ValidatorReturnType struct that contains
 * information about the validation. It has three members, \link ValidatorReturnType::errorMessages errorMessages\endlink
 * is the most important one. If that returns \c true for QString::isNull(), the validation has succeeded.
 *
 * So lets implement a custom validator that can check for a specific value to be set. (Maybe not a realistic example, but
 * it should be enough for demonstration.)
 *
 * \code{.cpp}
 * #include <Cutelyst/Plugins/Utils/ValidatorRule>
 *
 * namespace Cutelyst {
 *
 * class MyValidator : public ValidatorRule
 * {
 * public:
 *     // field: name of the input field
 *     // compareValue: our custom value we want compare
 *     // messages: struct containing custom messages
 *     // defValKey: name of a stash key containing a default value if input field is empty
 *     MyValidator::MyValidator(const QString &field,
 *                              const QString &compareValue,
 *                              const ValidatorMessages &messages = ValidatorMessages(),
 *                              const QString &defValKey = QString());
 *
 *     ~MyValidator();
 *
 *     // this will contain the validation logic and should return
 *     // a ValidatorResult with a null errorMessage string on success
 *     ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;
 *
 * protected:
 *     // we want to have a generic error message
 *     QString validationError(Context *c) const override;
 *
 * private:
 *     // storing our comparison value
 *     QString m_compareValue;
 * };
 *
 * }
 *
 * using namespace Cutelyst;
 *
 * MyValidator::MyValidator(const QString &field, const QString &compareValue, const ValidatorMessages &messages, const QString &defValKey) :
 *     Cutelyst::ValidatorRule(field, messages, defValKey), m_compareValue(compareValue)
 * {
 * }
 *
 * MyValidator::~MyValidator()
 * {
 * }
 *
 * // when reimplementing the validate function, keep in mind, that a null errorMessage
 * // string in the ValidatorReturnType is seen as successful validation, everything
 * // else will be seen as an error
 * ValidatorReturnType MyValidator::validate(Context *c, const ParamsMultiMap &params) const
 * {
 *     ValidatorReturnType result;
 *
 *     // lets get the field value
 *     const QString v = value(params);
 *
 *     // if our comparision value is empty, the validation should fail and we want
 *     // to return an error message according to this situation
 *     if (m_compareValue.isEmpty()) {
 *         result.errorMessage = validationDataError(c);
 *     } else {
 *
 *         // if the value is empty or the field is missing, the validation should succeed,
 *         // because we already have the required validators for that purpose
 *         // than we will compare our values and if they are not the same, we
 *         // will return an error string
 *         if (!v.isEmpty() && (m_compareValue != v)) {
 *              result.errorMessage = validationError(c);
 *         } else {
 *              result.value.setValue(v);
 *         }
 *     }
 *
 *     // now let's return our result, if the errorMessage member is null, validation was successfull
 *     return result;
 * }
 *
 *
 * QString MyValidator::genericValidationError(Context *c) const
 * {
 *     QString error;
 *     const QString _label = label(c);
 *     // if no label is set, we will return a shorter error message
 *     if (_label.isEmpty()) {
 *          c->translate("MyValidator", "Must contain this value: %1").arg(m_compareValue);
 *     } else {
 *          c->translate("MyValidator", "The %1 field must contain the following value: %2").arg(_label, m_compareValue);
 *     }
 *     return error;
 * }
 * \endcode
 *
 * That's it. Now you can use your own validator rule in the main Validator.
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorRule
{
public:
    /*!
     * \brief Constructs a new ValidatorRule with the given parameters.
     * \param field         Name of the field to validate.
     * \param messages      Custom error messages if validation fails.
     * \param defValKey     \link Context::stash() Stash \endlink key containing a default value if input field is empty. This value will \b NOT be validated.
     */
    ValidatorRule(const QString &field, const ValidatorMessages &messages = ValidatorMessages(), const QString &defValKey = QString());

    /*!
     * \brief Deconstructs the ValidatorRule.
     */
    virtual ~ValidatorRule();

protected:
    const QScopedPointer<ValidatorRulePrivate> d_ptr;
    /*!
     * \internal
     * \brief Constructs a new ValidatorRule object with the given private class.
     */
    ValidatorRule(ValidatorRulePrivate &dd);

    /*!
     * \brief Starts the validation and returns the result.
     *
     * This is the main function to reimplement when writing a custom validator. When reimplementing this function in a class
     * derived from ValidatorRule, you have to return an empty \link ValidatorReturnType::errorMessage errorMessage\endlink
     * if validation succeeded and the corresponding error if it fails. There are currently three error functions that should
     * be used for different error cases:
     *
     * \li validationError() - if validation itself fails
     * \li validationDataError() - if there is a problem with missing or invalid validation data, like comparison values
     * \li parsingError() - if the parsing of an input data fails in a validator that not originally checks the parsing, but
     * the parsed result
     *
     * If validation succeeded, you should put the extracted and validated value into the ValidatorReturnType::value.
     * After the validation you can get the extracted values from ValidatorResult::values().
     *
     * <h3>Example</h3>
     * \code{.cpp}
     * ValidatorReturnType MyValidator::validate(Context *c, const ParamsMultiMap &params) const
     * {
     *     ValidatorReturnType result;
     *
     *     if (!m_myComparisonDate.isValid()) {
     *         result.errorMessage = validationDataError(c);
     *     } else {
     *         const QString v = value(params);
     *         const QDate inputDate = QDate::fromString(v, Qt::ISODate);
     *         if (!inputDate.isValie()) {
     *             result.errorMessage = parsingError(c);
     *         } else {
     *             if (inputDate > m_myComparisonDate) {
     *                 result.value.setValue<QDate>(inputDate);
     *             } else {
     *                 result.errorMessage = validationError(c);
     *             }
     *         }
     *     }
     *
     *     return result;
     * }
     * \endcode
     */
    virtual ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const = 0;

    /*!
     * \brief Returns the name of the field to validate.
     * \return The name of the field to validate that has been set in the constructor.
     */
    QString field() const;

    /*!
     * \brief Returns the human readable field label used for generic error messages.
     * The label can be set in the ValidatorMessages on the constructor.
     * \return Human readable field label used for generic error messages.
     */
    QString label(Context *c) const;

    /*!
     * \brief Returns the value of the field from the input \a params.
     */
    QString value(const ParamsMultiMap &params) const;

    /*!
     * \brief Returns true if the field value should be trimmed before validation.
     *
     * By default, this will return \c true and all input values will be trimmed before validation to
     * remove whitespaces from the beginning and the end.
     */
    bool trimBefore() const;

    /*!
     * \brief Returns a descriptive error message if validation failed.
     *
     * This will either return the \a customValidationError message provided via the ValidatorMessages
     * in the \a messages argument of the constructor or the message returned by genericValidationError()
     * if there is no \a customValidationError message availabe.
     *
     * When writing a new ValidatorRule, use this in your reimplementaion of validate() if validation
     * failed.
     *
     * The pointer to the current Context \a c will be used to \link ValidatorMessages translate error strings\endlink.
     * If you have some more data to use for the error messages, put them into \a errorData.
     */
    QString validationError(Context *c, const QVariant &errorData = QVariant()) const;

    /*!
     * \brief Returns a generic error mesage if validation failed.
     *
     * If you want to have a more specifc generic validation error message for your validator
     * if validation fails, reimplment this your derived class. The default implementation simply
     * returns a maybe translated version of \c "The input data in the “%1” field is not acceptable."
     * if there has been a \link label() label\endlink set or \c "The input data is not acceptable."
     * if the \link label() label\endlink is empty.
     *
     * The pointer to the current Context \a c can be used to \link ValidatorMessages translate error strings\endlink.
     * If you have some more data to use for the error messages, put them into \a errorData.
     *
     * <h3>Example implementation</h3>
     * \code{.cpp}
     * QString MyValidator::genericValidationError(Context *c) const
     * {
     *     QString error;
     *     const QString _label = label(c);
     *     // if no label is set, we will return a shorter error message
     *     if (_label.isEmpty()) {
     *          c->translate("MyValidator", "Must contain this value: %1").arg(m_compareValue);
     *     } else {
     *          c->translate("MyValidator", "The %1 field must contain the following value: %2").arg(_label, m_compareValue);
     *     }
     *     return error;
     * }
     * \endcode
     */
    virtual QString genericValidationError(Context *c, const QVariant &errorData = QVariant()) const;

    /*!
     * \brief Returns an error message if an error occurred while parsing input.
     *
     * This will either return the \a customParsingError message provided via the ValidatorMessages
     * in the \a messages argument of the constructor or the message returned by genericValidationError()
     * if there is no \a customParsingError message availabe.
     *
     * When writing a new ValidatorRule, use this in your reimplementation of validate() if parsing of
     * input data fails.
     *
     * The pointer to the current Context \a c will be used to \link ValidatorMessages translate error strings\endlink.
     * If you have some more data to use for the error messages, put them into \a errorData.
     */
    QString parsingError(Context *c, const QVariant &errorData = QVariant()) const;

    /*!
     * \brief Returns a generic error message if an error occures while parsing input.
     *
     * If you want to have a more specific generic parsing error message for your validator
     * if parsing of input data failes, reimplement this in your derived class. The default implementation simply
     * returns a maybe translated version of \c "The input data in the “%1“ field could not be parsed."
     * if there has been a \link label() label\endlink set or \c "The input data could not be parsed."
     * if the \link label() label\endlink is empty.
     *
     * The pointer to the current Context \a c can be used to \link ValidatorMessages translate error strings\endlink.
     * If you have some more data to use for the error messages, put them into \a errorData.
     *
     * <h3>Example implementation</h3>
     * \code{.cpp}
     * QString MyValidator::genericParsingError(Context *c) const
     * {
     *     QString error;
     *     const QString _label = label(c);
     *     // if no label is set, we will return a shorter error message
     *     if (_label.isEmpty()) {
     *          c->translate("MyValidator", "Could not be parsed into a valid date.");
     *     } else {
     *          c->translate("MyValidator", "The value of the %1 field could not be parsed into a valid date.").arg(_label);
     *     }
     *     return error;
     * }
     * \endcode
     */
    virtual QString genericParsingError(Context *c, const QVariant &errorData = QVariant()) const;

    /*!
     * \brief Returns an error message if any validation data is missing or invalid.
     *
     * This will either return the \a customValidationDataError message provided via the ValidatorMessages
     * in the \a messages argument of the contstructor or the message returned by genericValidationDataError()
     * if there is no \a customValidationDataError message available.
     *
     * When writing a new ValidatorRule, use this in your reimplementation of validate() if validation
     * data like compare values are missing or invalid.
     *
     * The pointer to the current Context \a c will be used to \link ValidatorMessages translate error strings\endlink.
     * If you have some more data to use for the error messages, put them into \a errorData.
     */
    QString validationDataError(Context *c, const QVariant &errorData = QVariant()) const;

    /*!
     * \brief Returns a generic error message if any validation data is missing or invalid.
     *
     * If you want to have a more specific generic validation data error message for your validator
     * if data needed for the validation is missing or invalid, reimplement this in your derived class.
     * The default implementation simply returns a maybe translated version of \c "Missing or invalid validation data for the “%1” field."
     * if there has been a \link label() label\endlink set or \c "Missing or invalid validation data."
     * if the \link label() label\endlink is empty.
     *
     * The pointer to the current Context \a c can be used to \link ValidatorMessages translate error strings\endlink.
     * If you have some more data to use for the error messages, put them into \a errorData.
     *
     * <h3>Example implementation</h3>
     * \code{.cpp}
     * QString MyValidator::genericValidationDataError(Context *c) const
     * {
     *     QString error;
     *     const QString _label = label(c);
     *     // if no label is set, we will return a shorter error message
     *     if (_label.isEmpty()) {
     *          c->translate("MyValidator", "There is no value to compare against.");
     *     } else {
     *          c->translate("MyValidator", "For the “%1” field there is no value to compare against.").arg(_label);
     *     }
     *     return error;
     * }
     * \endcode
     */
    virtual QString genericValidationDataError(Context *c, const QVariant &errorData = QVariant()) const;

    /*!
     * \brief I a \a defValKey has been set in the constructor, this will try to get the default value from the stash and put it into the result.
     * \param c             Current Context to get the default value from.
     * \param result        The result struct to put the default value in.
     * \param validatorName Name of the validator used for logging.
     */
    void defaultValue(Context *c, ValidatorReturnType *result, const char *validatorName) const;

private:
    Q_DECLARE_PRIVATE(ValidatorRule)
    Q_DISABLE_COPY(ValidatorRule)

    /*!
     * \internal
     * \brief Sets the translation context used for custom messages.
     * \param trContext The name of the context.
     */
    void setTranslationContext(QLatin1String trContext);

    /*!
     * \internal
     * \brief Set to \c false to not trim input value before validation.
     *
     * By default, this value is set to \c true and all input values will be \link QString::trimmed() trimmed\endlink
     * before validation to remove whitespaces from the beginning and the end.
     *
     * \sa trimBefore()
     */
    void setTrimBefore(bool trimBefore);

    friend class Validator;
    friend class ValidatorPrivate;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORRULE_H
