/*
 * SPDX-FileCopyrightText: (C) 2017-2025 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORRULE_H
#define CUTELYSTVALIDATORRULE_H

#include <Cutelyst/Plugins/Utils/validator_export.h>
#include <Cutelyst/context.h>
#include <Cutelyst/paramsmultimap.h>
#include <functional>

#include <QLoggingCategory>
#include <QPointer>
#include <QScopedPointer>
#include <QVariant>

Q_DECLARE_LOGGING_CATEGORY(C_VALIDATOR)

namespace Cutelyst {

/**
 * \ingroup plugins-utils-validator
 * \defgroup plugins-utils-validator-rules Rules
 * \brief Classes providing rules to validate input data.
 *
 * All validator rule classes are derived from ValidatorRule and are meant to be used as part of
 * Validator. Read the documentation of Validator to learn more about how to use the Validator and
 * the rules. If you want to write your own validator rule, create a new class that is derived from
 * ValidatorRule and reimplement the validate function.
 *
 * There are three constructor arguments that are common to almost all validator rules: the name of
 * the field to validate (mandatory), a struct containing translatable ValidatorMessages (optional)
 * and a \link Context::stash() stash\endlink key name that contains a default value if the input
 * field is not available (optional available where it makes sense).
 *
 * Some validators export their validation logic in a <a href="#func-members">static member
 * function</a> so that it can be used without creating a Validator.
 */

class Context;

/**
 * \ingroup plugins-utils-validator
 * \headerfile "" <Cutelyst/Plugins/Utils/ValidatorRule>
 * \brief Contains the result of a single input parameter validation.
 *
 * For information about the possible values of \link ValidatorReturnType::value value\endlink and
 * \link ValidatorReturnType::extra extra\endlink see the documentation of the respective
 * \link plugins-utils-validator-rules validator rule\endlink.
 */
struct CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorReturnType {
    QString errorMessage; /**< Contains a human readable error message if validation failed that
                             should provide information about the reason the validation failed. If
                             QString::isNull() returns \c true for this, the validation has
                             succeeded and isValid() will alsor return \c true. */
    QVariant value;       /**< Might contain the extracted and possibly converted value of the \link
                             ValidatorRule::value() input parameter\endlink if validation succeeded.
                             For information about the possible values see the documentation of the
                             respective \link plugins-utils-validator-rules validator\endlink. */
    QVariant extra; /**< Might contain extra data provided by the validator. For information about
                       the possible values see the documentation of the respective \link
                       plugins-utils-validator-rules validator\endlink. */

    /**
     * Returns \c true if validation succeeded what means that errorMessage is a null string.
     */
    explicit operator bool() const noexcept { return errorMessage.isNull(); }

    /**
     * Returns \c true if validation succeeded what means that errorMessage is a null string.
     */
    [[nodiscard]] bool isValid() const noexcept { return errorMessage.isNull(); }
};

/**
 * \related ValidatorRule
 * \brief Void callback function for validator rules that processes the ValidatorReturnType.
 *
 * Has to be in the form void function(ValidatorReturnType &&result).
 */
using ValidatorRtFn = std::function<void(ValidatorReturnType &&result)>;

/**
 * \ingroup plugins-utils-validator
 * \headerfile "" <Cutelyst/Plugins/Utils/ValidatorRule>
 * \brief Stores custom error messages and the input field label.
 *
 * This struct is used by ValidatorRule derived classes to store custom error messages
 * that are also translatable. To make the messages translatable, use QT_TRANSLATE_NOOP()
 * or QT_TRID_NOOP() so that the message can be dynamically translated by the current Context
 * either via Context::translate() or Context::qtTrId(). If you want to omit a custom message,
 * simply use a \c nullptr for it. For custom messages that are not set, the ValidatorRule class
 * will return a generic message that will also be translated if %Cutelyst has a translation
 * file for the \link Context::locale() current language in the context\endlink.
 *
 * Note that you should not mix id based and non-id based translations, so you should not mix
 * QT_TRID_NOOP and QT_TRANSLATE_NOOP.
 *
 * If using QT_TRANSLATE_NOOP you have to set the translation context to the constructor of
 * Validator. Id based translations do not use a translation context. So, if you omit the
 * translation context on the constructor of Validator, internally Context::qtTrId() will be
 * used to find the translated string.
 *
 * \sa \ref translations
 *
 * <h3>Usage example</h3>
 * \code{.cpp}
 * void MyController::do_form(Context *c)
 * {
 *   static Validator v1({
 *     new ValidatorAccepted(QStringLiteral("usage_terms"),
 *                           // empty label and translatable validation error message
 *                           ValidatorMessages(nullptr,
 *                                             QT_TRANSLATE_NOOP("MyController",
 *                                                               "Please accept our terms of "
 *                                                               "usage to finish your "
 *                                                               "registration.")
 *                                            ),
 *
 *     new ValidatorEmail(QStringLiteral("email"), false, ValidatorEmail::Valid,
 *                       // only the label will be translated and inserted into generic error
 *                       // messages of the validator rule
 *                       ValidatorMessages(QT_TRANSLATE_NOOP("MyController",
 *                                                           "Your Email Address"))
 *                       ),
 *
 *     new ValidatorDate(QStringLiteral("birthday"),
 *                       // here we also use a translatable version of the date format
 *                       QT_TRANSLATE_NOOP("MyControler", "yyyy-MM-dd"),
 *                       // this one will use a translated label for generic parsing and validation
 *                       // data error messages and a custom translatable error message for failed
 *                       // validations
 *                       ValidatorMessages(QT_TRANSLATE_NOOP("MyController",
 *                                                           "Your day of birth"),
 *                                         QT_TRANSLATE_NOOP("MyController",
 *                                                           "Please enter a valid date of birth"))
 *                       ),
 *
 *     // this uses a default constructed ValidatorMessages struct where no custom messages
 *     // have been set, so all error messages will come from the generic validator messages
 *     // without a label
 *     new ValidatorRequired(QStringLiteral("password"));
 *
 *     // this is the context that will be used to obtain translations,
 *     // it has to be the same one you use in your validator messages
 *     // QT_TRANSLATE_NOOP(). If you used QT_TRID_NOOP() you can omit
 *     // this
 *   }, "MyController");
 * }
 * \endcode
 */
struct CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorMessages {
    /**
     * Constructs a default %ValidatorMessages object with all custom messages disabled.
     */
    ValidatorMessages() = default;
    /**
     * \brief Constructs a new %ValidatorMessages object with the given parameters.
     *
     * \param customLabel               User visible label for the input field. Should be the same
     *                                  as used on the frontend visible to the user. Will be used
     *                                  by generic error messages if set.
     * \param customValidationError     Custom error message if the validation fails.
     * \param customParsingError        Custom error message if the input value could not be parsed.
     * \param customValidationDataError Custom error message if validation data is missing or
     *                                  invalid.
     */
    explicit ValidatorMessages(const char *customLabel,
                               const char *customValidationError     = nullptr,
                               const char *customParsingError        = nullptr,
                               const char *customValidationDataError = nullptr)
        : label(customLabel)
        , validationError(customValidationError)
        , parsingError(customParsingError)
        , validationDataError(customValidationDataError)
    {
    }
    const char *label = nullptr; /**< Field label used for generating generic error messages. */
    const char *validationError     = nullptr; /**< Custom validation error messages. */
    const char *parsingError        = nullptr; /**< Custom parsing error message. */
    const char *validationDataError = nullptr; /**< Custom validation data error message. */
};

class ValidatorRulePrivate;

/**
 * \ingroup plugins-utils-validator
 * \headerfile "" <Cutelyst/Plugins/Utils/ValidatorRule>
 * \brief Base class for all validator \link plugins-utils-validator-rules rules\endlink.
 *
 * This class can not be used on it’s own, you have to create a derived class from it that
 * implements your validator logic. Or use one of the \link plugins-utils-validator-rules already
 * existing validator rules\endlink.
 *
 * \par Writing a custom validator
 *
 * If you want to implement your own validator logic to use with Validator, you have to create a
 * class that derives from ValidatorRule. The simplest implementation only needs a constructor and
 * a reimplementation of the validate() function. But for more comfort and usability, you should
 * also reimplement genericValidationError(). If your validator parses the input into a specific
 * type to validate it and/or if you are using additional parameters, you may also want to
 * reimplement genericParsingError() and genericValidationDataError() to return more appropriate
 * generic error messages.
 *
 * The most important parameter for every validator rule is the name of the \a field to validate.
 * So your own validator should require that field in the constructor. For better error messages
 * you should also add an optional paramter to set custom ValidatorMessages if validation fails.
 *
 * In the validation logic implemented in the validate() function you have to return a
 * ValidatorReturnType struct that contains information about the validation. It has three members,
 * \link ValidatorReturnType::errorMessages errorMessages\endlink is the most important one. If
 * that returns \c true for QString::isNull(), the validation has succeeded.
 *
 * For validators that can be used in coroutine contexts, you also have to implement validateCb().
 * The callback function there takes a ValidatorReturnType struct as returned by validate(). So
 * for that the rules are the same as in the previous paragraph.
 *
 * So lets implement a custom validator that can check for a specific value to be set. (Maybe not
 * a realistic example, but it should be enough for demonstration.)
 *
 * \code{.cpp}
 * #include <Cutelyst/Plugins/Utils/ValidatorRule>
 *
 * using namespace Cutelyst;
 *
 * class MyValidator : public ValidatorRule
 * {
 * public:
 *     // field: name of the input field
 *     // compareValue: our custom value we want compare
 *     // messages: struct containing custom messages
 *     // defValKey: name of a stash key containing an optional default value if input field
 *     //            is empty
 *     MyValidator::MyValidator(const QString &field,
 *                              const QString &compareValue,
 *                              const ValidatorMessages &messages = {},
 *                              const QString &defValKey = {});
 *
 *     ~MyValidator();
 *
 * protected:
 *     // this will contain the validation logic and should return
 *     // a ValidatorResult with a null errorMessage string on success
 *     ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;
 *
 *     // this will contain the validaion logic for asynchronous validation
 *     // the callback cb will take the ValidatorResult with a null errorMessage string on success
 *     void validateCb(Context *c, const ParamsMultiMap &params, ValidatorRtFn cb) const override;
 *
 *     // we want to have a generic error message
 *     QString genericValidationError(Context *c,
 *                                    const QVariant &errorData = {}) const override;
 *
 * private:
 *     // storing our comparison value
 *     QString m_compareValue;
 * };
 *
 *
 * MyValidator::MyValidator(const QString &field,
 *                          const QString &compareValue,
 *                          const ValidatorMessages &messages,
 *                          const QString &defValKey)
 *      : ValidatorRule(field, messages, defValKey, "MyValidator"),
 *        m_compareValue(compareValue)
 * {
 * }
 *
 * MyValidator::~MyValidator() = default;
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
 *     // if our comparison value is empty, the validation should fail and we want
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
 *     // now let's return our result, if the errorMessage member is null,
 *     // validation was successfull
 *     return result;
 * }
 *
 * // This will be used for async validation. For the ValidatorReturnType that has to be given
 * // to the callback function cb, keep the notes from the synchronous implementation from above
 * // in mind.
 * void MyValidator::validateCb(Context *c, const ParamsMultiMap &params, ValidatorRtFn cb) const
 * {
 *     // Note that for demonstration purposes we use a more complex approach.
 *     // For simple validators like this you normally can simply do the following:
 *     // cb(validate(c, params));
 *
 *     // lets get the field value
 *     const QString v = value(params);
 *
 *     // if our comparison value is empty, the validation should fail and we want
 *     // to return an error message according to this situation
 *     if (m_compareValue.isEmpty()) {
 *         cb({.errorMessage = validationDataError(c)});
 *     } else {
 *
 *         // if the value is empty or the field is missing, the validation should succeed,
 *         // because we already have the required validators for that purpose
 *         // than we will compare our values and if they are not the same, we
 *         // will return an error string
 *         if (!v.isEmpty() && (m_compareValue != v)) {
 *             cb({.errorMessage = validationError(c)});
 *         } else {
 *             cb({.errorMessage = {}, .value = v})
 *         }
 *     }
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
 *          c->translate("MyValidator",
 *                       "The %1 field must contain the following value: %2")
 *            .arg(_label, m_compareValue);
 *     }
 *     return error;
 * }
 * \endcode
 *
 * That’s it. Now you can use your own validator rule in the main Validator.
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorRule
{
public:
    /**
     * Constructs a new %ValidatorRule object with the given parameters.
     *
     * \param field         Name of the field to validate.
     * \param messages      Custom error messages if validation fails.
     * \param defValKey     \link Context::stash() Stash \endlink key containing a default value if
     *                      input field is empty. This value will \b NOT be validated.
     * \param vaidatorName  Name of the validator used for debug output.
     */
    explicit ValidatorRule(const QString &field,
                           const ValidatorMessages &messages = {},
                           const QString &defValKey          = {},
                           QByteArrayView validatorName      = nullptr);

    /**
     * \brief Deconstructs the ValidatorRule.
     */
    virtual ~ValidatorRule();

protected:
    // shared d-pointer
    // NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
    const std::unique_ptr<ValidatorRulePrivate> d_ptr;
    /**
     * \internal
     * Constructs a new ValidatorRule object with the given private class.
     */
    explicit ValidatorRule(ValidatorRulePrivate &dd);

    /**
     * Starts the validation and returns the result.
     *
     * This is the main function to reimplement when writing a custom validator. When reimplementing
     * this function in a class derived from ValidatorRule, you have to return an empty \link
     * ValidatorReturnType::errorMessage errorMessage\endlink if validation succeeded and the
     * corresponding error if it fails. There are currently three error functions that should be
     * used for different error cases:
     *
     * \li validationError() - if validation itself fails
     * \li validationDataError() - if there is a problem with missing or invalid validation data,
     * like comparison values
     * \li parsingError() - if the parsing of an input data fails in a validator that not
     * originally checks the parsing, but the parsed result
     *
     * If validation succeeded, you should put the extracted and validated value into the
     * ValidatorReturnType::value. After the validation you can get the extracted values from
     * ValidatorResult::values().
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
     *         if (!inputDate.isValid()) {
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

    /**
     * Starts the validation and writes the \a result to the callback \a cb.
     *
     * This is the main function to reimplement when writing a custom validator that can be used
     * in a coroutine. When reimplementing this function in a class derived from ValidatorRule,
     * you have to set an empty \link ValidatorReturnType::errorMessage errorMessage\endlink if
     * validation succeeded and the corresponding error if it fails. There are currently three
     * error functions that should be used for different error cases:
     *
     * \li validationError() - if validation itself fails
     * \li validationDataError() - if there is a problem with missing or invalid validation data,
     * like comparison values
     * \li parsingError() - if the parsing of an input data fails in a validator that not
     * originally checks the parsing, but the parsed result
     *
     * If validation succeeded, you should put the extracted and validated value into the
     * ValidatorReturnType::value. After the validation you can get the extracted values from
     * ValidatorResult::values().
     *
     * <h3>Example</h3>
     * \code{.cpp}
     * void MyValidator::validateCb(Context *c, const ParamsMultiMap &params, ValidatorRtFn cb)
     * const
     * {
     *     if (!m_myComparisonDate.isValid()) {
     *         cb({.errorMessage = validationDataError(c)});
     *     } else {
     *         const QString v = value(params);
     *         const QDate inputDate = QDate::fromString(v, Qt::ISODate);
     *         if (!inputDate.isValid()) {
     *             cb({.errorMessage = parsingError(c)});
     *         } else {
     *             if (inputDate > m_myComparisonDate) {
     *                 cb({.errorMessage = {}, .value = inputDate});
     *             } else {
     *                 cb({.errorMessage = validationError(c)});
     *             }
     *         }
     *     }
     * }
     * \endcode
     *
     * \since Cutelyst 5.0.0
     */
    virtual void validateCb(Context *c, const ParamsMultiMap &params, ValidatorRtFn cb) const;

    /**
     * Returns the name of the field to validate.
     */
    [[nodiscard]] QString field() const noexcept;

    /**
     * Returns the human readable field label used for generic error messages.
     * The label can be set in the ValidatorMessages on the constructor.
     */
    [[nodiscard]] QString label(const Context *c) const;

    /**
     * Returns the value of the field from the input \a params.
     */
    [[nodiscard]] QString value(const ParamsMultiMap &params) const;

    /**
     * Returns \c true if the field value should be trimmed before validation.
     *
     * By default, this will return \c true and all input values will be trimmed before validation
     * to remove whitespaces from the beginning and the end.
     */
    [[nodiscard]] bool trimBefore() const noexcept;

    /**
     * Returns a descriptive error message if validation failed.
     *
     * This will either return the \a customValidationError message provided via the
     * ValidatorMessages in the \a messages argument of the constructor or the message returned by
     * genericValidationError() if there is no \a customValidationError message availabe.
     *
     * When writing a new ValidatorRule, use this in your reimplementaion of validate() if
     * validation failed.
     *
     * The pointer to the current Context \a c will be used to \link ValidatorMessages translate
     * error strings\endlink. If you have some more data to use for the error messages, put them
     * into \a errorData.
     */
    [[nodiscard]] QString validationError(Context *c, const QVariant &errorData = {}) const;

    /**
     * Returns a generic error mesage if validation failed.
     *
     * If you want to have a more specifc generic validation error message for your validator
     * if validation fails, reimplment this in your derived class. The default implementation simply
     * returns a maybe translated version of \c "The input data in the “%1” field is not
     * acceptable." if there has been a \link label() label\endlink set or \c "The input data is not
     * acceptable." if the \link label() label\endlink is empty.
     *
     * The pointer to the current Context \a c can be used to \link ValidatorMessages translate
     * error strings\endlink. If you have some more data to use for the error messages, put them
     * into \a errorData.
     *
     * <h3>Example implementation</h3>
     * \code{.cpp}
     * QString MyValidator::genericValidationError(Context *c) const
     * {
     *     const QString _label = label(c);
     *     // if no label is set, we will return a shorter error message
     *     if (_label.isEmpty()) {
     *          return c->translate("MyValidator", "Must contain this value: %1")
     *                   .arg(m_compareValue);
     *     } else {
     *          return c->translate("MyValidator", "The %1 field must contain the following "
     *                                             "value: %2")
     *                   .arg(_label, m_compareValue);
     *     }
     * }
     * \endcode
     */
    virtual QString genericValidationError(Context *c, const QVariant &errorData = {}) const;

    /**
     * Returns an error message if an error occurred while parsing input.
     *
     * This will either return the \a customParsingError message provided via the ValidatorMessages
     * in the \a messages argument of the constructor or the message returned by
     * genericValidationError() if there is no \a customParsingError message availabe.
     *
     * When writing a new ValidatorRule, use this in your reimplementation of validate() if parsing
     * of input data fails.
     *
     * The pointer to the current Context \a c will be used to \link ValidatorMessages translate
     * error strings\endlink. If you have some more data to use for the error messages, put them
     * into \a errorData.
     */
    [[nodiscard]] QString parsingError(Context *c, const QVariant &errorData = {}) const;

    /**
     * Returns a generic error message if an error occures while parsing input.
     *
     * If you want to have a more specific generic parsing error message for your validator
     * if parsing of input data failes, reimplement this in your derived class. The default
     * implementation simply returns a maybe translated version of \c "The input data in the “%1“
     * field could not be parsed." if there has been a \link label() label\endlink set or \c "The
     * input data could not be parsed." if the \link label() label\endlink is empty.
     *
     * The pointer to the current Context \a c can be used to \link ValidatorMessages translate
     * error strings\endlink. If you have some more data to use for the error messages, put them
     * into \a errorData.
     *
     * <h3>Example implementation</h3>
     * \code{.cpp}
     * QString MyValidator::genericParsingError(Context *c) const
     * {
     *     const QString _label = label(c);
     *     // if no label is set, we will return a shorter error message
     *     if (_label.isEmpty()) {
     *          return c->translate("MyValidator", "Could not be parsed into a valid date.");
     *     } else {
     *          return c->translate("MyValidator",
     *                              "The value of the %1 field could not be parsed into "
     *                              "a valid date.")
     *                   .arg(_label);
     *     }
     * }
     * \endcode
     */
    virtual QString genericParsingError(Context *c, const QVariant &errorData = {}) const;

    /**
     * Returns an error message if any validation data is missing or invalid.
     *
     * This will either return the \a customValidationDataError message provided via the
     * ValidatorMessages in the \a messages argument of the contstructor or the message returned by
     * genericValidationDataError() if there is no \a customValidationDataError message available.
     *
     * When writing a new ValidatorRule, use this in your reimplementation of validate() if
     * validation data like compare values is missing or invalid.
     *
     * The pointer to the current Context \a c will be used to \link ValidatorMessages translate
     * error strings\endlink. If you have some more data to use for the error messages, put them
     * into \a errorData.
     */
    [[nodiscard]] QString validationDataError(Context *c, const QVariant &errorData = {}) const;

    /**
     * Returns a generic error message if any validation data is missing or invalid.
     *
     * If you want to have a more specific generic validation data error message for your validator
     * if data needed for the validation is missing or invalid, reimplement this in your derived
     * class. The default implementation simply returns a maybe translated version of \c "Missing or
     * invalid validation data for the “%1” field." if there has been a \link label() label\endlink
     * set or \c "Missing or invalid validation data." if the \link label() label\endlink is empty.
     *
     * The pointer to the current Context \a c can be used to \link ValidatorMessages translate
     * error strings\endlink. If you have some more data to use for the error messages, put them
     * into \a errorData.
     *
     * <h3>Example implementation</h3>
     * \code{.cpp}
     * QString MyValidator::genericValidationDataError(Context *c) const
     * {
     *     const QString _label = label(c);
     *     // if no label is set, we will return a shorter error message
     *     if (_label.isEmpty()) {
     *          return c->translate("MyValidator", "There is no value to compare against.");
     *     } else {
     *          return c->translate("MyValidator",
     *                              "For the “%1” field there is no value to compare against.")
     *                   .arg(_label);
     *     }
     * }
     * \endcode
     */
    virtual QString genericValidationDataError(Context *c, const QVariant &errorData = {}) const;

    /**
     * If a \a defValKey has been set in the constructor, this will try to get the default
     * value from the stash of context \a c and put it into the \a result.
     */
    void defaultValue(Context *c, ValidatorReturnType *result) const;

    /**
     * If a \a defValKey has been set in the constructor, this will try to the the default
     * value from the stash of context \a c and put it into the result written to the callback
     * function \a cb.
     *
     * \since Cutelyst 5.0.0
     */
    void defaultValue(Context *c, ValidatorRtFn cb) const;

    /**
     * Returns a string that can be used for debug output if validation fails.
     *
     * This returns something like <tt>MyValidator: Validation failed for field "my_field" at
     * MyController::myAction:</tt>
     */
    [[nodiscard]] QString debugString(const Context *c) const;

private:
    Q_DECLARE_PRIVATE(ValidatorRule) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    Q_DISABLE_COPY(ValidatorRule)

    /**
     * \internal
     * Sets the translation context \a trContext used for custom messages.
     */
    void setTranslationContext(const char *trContext) noexcept;

    /**
     * \internal
     * Set to \c false to not trim input value before validation.
     *
     * By default, this value is set to \c true and all input values will be \link
     * QString::trimmed() trimmed\endlink before validation to remove whitespaces from the
     * beginning and the end.
     *
     * \sa trimBefore()
     */
    void setTrimBefore(bool trimBefore) noexcept;

    friend class Validator;
    friend class ValidatorPrivate;
    friend class AsyncValidator;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORRULE_H
