/*
 * SPDX-FileCopyrightText: (C) 2017-2025 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORRESULT_H
#define CUTELYSTVALIDATORRESULT_H

#include <Cutelyst/Plugins/Utils/validator_export.h>
#include <Cutelyst/context.h>
#include <coroutine>
#include <functional>

#include <QJsonObject>
#include <QPointer>
#include <QSharedDataPointer>
#include <QString>
#include <QStringList>
#include <QVariantHash>

namespace Cutelyst {

class ValidatorResultPrivate;

/**
 * \ingroup plugins-utils-validator
 * \headerfile "" <Cutelyst/Plugins/Utils/ValdatorResult>
 * \brief Provides information about performed validations.
 *
 * %ValidatorResult will be returned by Validator when calling \link Validator::validate()
 * validate()\endlink on it. It contains information about occurred validation errors, like the
 * error strings of each failed validator and a list of fields where validation failed.
 *
 * Additionally to the error messages that occure if validation fails for one or more fields,
 * %ValidatorResult will also contain the extracted values from the input parameters. Use values()
 * to return all values or value() to return the value for a single field. Because there will be
 * only one value stored for each field, you should order your validators in a way that a validator
 * for a field comes last that converts the input QString into the required type. See the
 * documentation for the specific validator to see what type of data it returns.
 *
 * Some validators might even return more details about the validation result. This extra data can
 * be returned with the extras() method for all input parameters or with extra() for a single one.
 *
 * Beside the isValid() function, that returns \c true if the complete validation process was
 * successful and \c false if any of the validators failed, %ValidatorResult provides a bool
 * operator that makes it usable in \c if statements.
 *
 * \code{.cpp}
 * void MyController:do_form(Context *c)
 * {
 *     static Validator v({
 *                          new ValidatorRequired(QStringLiteral("birthday"),
 *                          new ValidatorDate(QStringLiteral("birthday")
 *                       });
 *     ValidatorResult r = v.validate(c);
 *     if (r) {
 *         // do stuff if validation was successful
 *         auto extractedValues = r.values();
 *     } else {
 *         // do other stuff if validation failed
 *         auto errors = r.errors();
 *     }
 *
 *
 *     // you can also compare the boolen value of the result directly
 *     // for example together with the Validator::FillStashOnError flag
 *     // to automatically fill the stash with error information and input values
 *     if (v.validate(Validator::FillStashOnError)) {
 *         // do stuff if validation was successful
 *     }
 * }
 * \endcode
 *
 * Validity is simply determined by the fact, that it does not contain any error information.
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorResult
{
public:
    /**
     * Constructs a new %ValidatorResult object.
     *
     * A newly constructed %ValidatorResult willl be \link isValid() valid\endlink by default,
     * because it does not contain any error information.
     */
    ValidatorResult();

    /**
     * Constructs a copy of \a other.
     */
    ValidatorResult(const ValidatorResult &other) noexcept;

    /**
     * Move-constructs a %ValidatorResult instance, making it point at the same object that
     * \a other was pointing to.
     */
    ValidatorResult(ValidatorResult &&other) noexcept;

    /**
     * Assigns \a other to this %ValidatorResult and returns a reference to this result.
     */
    ValidatorResult &operator=(const ValidatorResult &other) noexcept;

    /**
     * Move-assings \a other to this %ValidatorResult instance.
     */
    ValidatorResult &operator=(ValidatorResult &&other) noexcept;

    /**
     * Destroys the %ValidatorResult object.
     */
    ~ValidatorResult() noexcept;

    /**
     * Returns \c true if the validation was successful.
     *
     * \note A newly constructed %ValidatorResult will be valid by default.
     */
    [[nodiscard]] bool isValid() const noexcept;

    /**
     * Adds new error information to the internal QHash.
     *
     * \param field     Name of the input \link Request::parameters() parameter\endlink
     *                  that has validation errors.
     * \param message   Error message shown to the user.
     *
     * \sa errorString() errors() hasErrors()
     */
    void addError(const QString &field, const QString &message);

    /**
     * Returns a list of all error messages.
     */
    [[nodiscard]] QStringList errorStrings() const;

    /**
     * Returns a dictionary containing fields with errors.
     *
     * The rerunted hash contains the name of the input \link Request::parameters()
     * parameter\endlink as key and a list of validation errors for this parameter
     * in a QStringList as value.
     */
    [[nodiscard]] QHash<QString, QStringList> errors() const noexcept;

    /**
     * Returns a list of all error messages for an input \a field.
     *
     * If there were no errors for the \a field, the returned list will be empty.
     */
    [[nodiscard]] QStringList errors(const QString &field) const noexcept;

    /**
     * Returns \c true if the \a field has validation errors.
     *
     * \since Cutelyst 2.0.0
     */
    [[nodiscard]] bool hasErrors(const QString &field) const noexcept;

    /**
     * Returns the dictionray containing fields with errors as JSON object.
     *
     * This returns the same data as errors() but converted into a JSON object
     * that has the field names as keys and the values will be a JSON array of
     * strings containing the errors for the field.
     *
     * \sa errors() errorStrings()
     * \since Cutelyst 1.12.0
     */
    [[nodiscard]] QJsonObject errorsJsonObject() const;

    /**
     * \brief Returns a list of fields with errors.
     * \since Cutelyst 1.12.0
     */
    [[nodiscard]] QStringList failedFields() const;

    /**
     * Returns \c true if the validation was successful.
     *
     * \note A newly constructed ValidatorResult will be valid by default.
     */
    explicit operator bool() const noexcept { return isValid(); }

    /**
     * Returns the values that have been extracted by the validators.
     *
     * Returns a QVariantHash where the key is the name of the input \link Request::parameters()
     * parameter\endlink and the value contains the value extracted from the input parameter.
     * Have a look at the documentation of the specific validator to see what kind of extracted
     * value they will provide.
     *
     * \sa value() addValue()
     * \since Cutelyst 2.0.0
     */
    [[nodiscard]] QVariantHash values() const noexcept;

    /**
     * Returns the extracted value for the input \a field.
     *
     * The returned value will be a QVariant. If there is no value for the \a field, the
     * returned QVariant will be default constructed. Have a look at the documentation of the
     * specific validator to see what kind of extracted value they will provide.
     *
     * \sa values() addValue()
     * \since Cutelyst 2.0.0
     */
    [[nodiscard]] QVariant value(const QString &field) const noexcept;

    /**
     * Adds a new \a value extracted from the specified input \a field.
     *
     * \sa values() value()
     * \since Cutelyst 2.0.0
     */
    void addValue(const QString &field, const QVariant &value);

    /**
     * Returns all extra data that has been extracted by the validators.
     *
     * Returns a QVariantHash where the key is the name of the input \link Request::parameters()
     * parameter\endlink and the value contains the extra data for that field. Have a look at the
     * documentation of the specific validators to see what kind of extra data they might generate.
     *
     * \sa extra() addExtra()
     * \since Cutelyst 2.0.0
     */
    [[nodiscard]] QVariantHash extras() const noexcept;

    /**
     * Returns the extra data for the input \a field.
     *
     * Returns a QVariant containing extra data generated by the validators. If the \a
     * field does not have any extra data, a default constructed QVariant will be returned. Have a
     * look at the documentation of the specific validators to see what kind of extra data they
     * might generate.
     *
     * \sa extras() addExtra()
     * \since Cutelyst 2.0.0
     */
    [[nodiscard]] QVariant extra(const QString &field) const noexcept;

    /**
     * Adds new \a extra data that came up when validating the input \a field.
     *
     * \sa extras() extra()
     * \since Cutelyst 2.0.0
     */
    void addExtra(const QString &field, const QVariant &extra);

private:
    QSharedDataPointer<ValidatorResultPrivate> d;
};

/**
 * \ingroup plugins-utils-validator
 * \headerfile "" <Cutelyst/Plugins/Utils/ValdatorResult>
 * \brief Coroutine awaitable for ValidatorResult.
 * \since Cutelyst 5.0.0
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT AwaitedValidatorResult
{
public:
    bool await_ready() const noexcept { return m_hasResult; }

    bool await_suspend(std::coroutine_handle<> h) noexcept
    {
        m_handle = h;
        if (m_receiver) {
            m_destroyConn = QObject::connect(m_receiver, &QObject::destroyed, [h, this] {
                m_result.addError(
                    QString(), QStringLiteral("Internal Server Error: the context was destroyed."));
                m_hasResult = true;
                h.resume();
            });
        }

        return !await_ready();
    }

    ValidatorResult await_resume() { return m_result; }

    explicit AwaitedValidatorResult(Context *c)
        : m_receiver{c}
    {
        callback = [this](const ValidatorResult &result) {
            m_result    = result; // cppcheck-suppress useInitializationList
            m_hasResult = true;

            if (m_handle) {
                m_handle.resume();
            }
        };
    }

    ~AwaitedValidatorResult() { QObject::disconnect(m_destroyConn); }

protected:
    friend class Validator;
    std::function<void(const ValidatorResult &result)> callback;

private:
    QMetaObject::Connection m_destroyConn;
    QPointer<Context> m_receiver;
    ValidatorResult m_result;
    std::coroutine_handle<> m_handle;
    bool m_hasResult{false};
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORRESULT_H
