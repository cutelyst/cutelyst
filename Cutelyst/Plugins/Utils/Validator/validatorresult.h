/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORRESULT_H
#define CUTELYSTVALIDATORRESULT_H

#include <Cutelyst/cutelyst_global.h>

#include <QJsonObject>
#include <QSharedDataPointer>
#include <QString>
#include <QStringList>
#include <QVariantHash>

namespace Cutelyst {

class ValidatorResultPrivate;

/*!
 * \ingroup plugins-utils-validator
 * \class ValidatorResult validatorresult.h <Cutelyst/Plugins/Utils/ValdatorResult>
 * \brief Provides information about performed validations.
 *
 * %ValidatorResult will be returned by Validator when calling \link Validator::validate() validate()\endlink on it.
 * It contains information about occurred validation errors, like the error strings of each failed validator and a
 * list of fields where validation failed.
 *
 * Additionally to the error messages that occure if validation fails for one or more fields, %ValidatorResult will
 * also contain the extracted values from the input parameters. Use values() to return all values or value() to
 * return the value for a single field. Because there will be only one value stored for each field, you should order
 * your validators in a way that a validator for a field comes last that converts the input QString into the
 * wanted type. See the documentation for the specific validator to see what type of data it returns.
 *
 * Some validators might even return more details about the validation result. This extra data can be returned with
 * the extras() method for all input parameters or with extra() for a single one.
 *
 * Beside the isValid() function, that returns \c true if the complete validation process was successful and \c false
 * if any of the validators failed, %ValidatorResult provides a bool operator that makes it usable in \c if statements.
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
    /*!
     * \brief Constructs a new %ValidatorResult.
     *
     * A newly constructed %ValidatorResult willl be \link isValid() valid\endlink by default,
     * because it does not contain any error information.
     */
    ValidatorResult();

    /*!
     * \brief Constructs a copy of \a other.
     */
    ValidatorResult(const ValidatorResult &other);

    /*!
     * \brief Assigns \a other to this %ValidatorResult.
     */
    ValidatorResult &operator=(const ValidatorResult &other);

    /*!
     * \brief Deconstructs the %ValidatorResult.
     */
    ~ValidatorResult();

    /*!
     * \brief Returns \c true if the validation was successful.
     *
     * \note A newly constructed %ValidatorResult will be valid by default.
     */
    bool isValid() const;

    /*!
     * \brief Adds new error information to the internal QHash.
     * \param field     Name of the input \link Request::parameters() parameter\endlink that has validation errors.
     * \param message   Error message shown to the user.
     * \sa errorString() errors() hasErrors()
     */
    void addError(const QString &field, const QString &message);

    /*!
     * \brief Returns a list of all error messages.
     * \return A list of all error messages from every failed ValidatorRule.
     */
    QStringList errorStrings() const;

    /*!
     * \brief Returns a dictionary containing fields with errors.
     * \return A QHash containing the name of the input \link Request::parameters() parameter\endlink as key and
     * a list of validation errors for this parameter in a QStringList as value.
     */
    QHash<QString, QStringList> errors() const;

    /*!
     * \brief Returns a list of all error messages for an input \a field.
     * \param field Name of the field to return error messages for.
     * \return List of error message strings. If there were no errors for the \a field, the
     * returned list will be empty.
     * \sa errorStrings()
     */
    QStringList errors(const QString &field) const;

    /*!
     * \brief Returns \c true if the \a field has validation errors.
     * \param field Name of the input field to check.
     * \return \c true if the \a field has error messages, \c false otherwise.
     * \sa \link errors(const QString &field) errors()\endlink
     * \since Cutelyst 2.0.0
     */
    bool hasErrors(const QString &field) const;

    /*!
     * \brief Returns the dictionray containing fields with errors as JSON object.
     *
     * This returns the same data as errors() but converted into a JSON object
     * that has the field names as keys and the values will be a JSON array of
     * strings containing the errors for the field.
     *
     * \sa errors() errorStrings()
     * \since Cutelyst 1.12.0
     */
    QJsonObject errorsJsonObject() const;

    /*!
     * \brief Returns a list of fields with errors.
     * \return A list of of  input \link Request::parameters() parameter\endlink names that have validation errors.
     * \since Cutelyst 1.12.0
     */
    QStringList failedFields() const;

    /*!
     * \brief Returns \c true if the validation was successful.
     *
     * \note A newly constructed ValidatorResult will be valid by default.
     */
    explicit operator bool() const
    {
        return isValid();
    }

    /*!
     * \brief Returns the values that have been extracted by the validators.
     * \return A QVariantHash where the key is the name of the input \link Request::parameters() parameter\endlink and
     * the value contains the value extracted from the input parameter. Have a look at the documentation of the specific
     * validator to see what kind of extracted value they will provide.
     * \sa value() addValue()
     * \since Cutelyst 2.0.0
     */
    QVariantHash values() const;

    /*!
     * \brief Returns the extracted value for the input \a field.
     * \param field Name of the input \link Request::parameters() parameter\endlink to get the value for.
     * \return The extracted value in a QVariant. If there is no value for the \a field, the returned QVariant will
     * be default constructed. Have a look at the documentation of the specific validator to see what kind of extracted
     * value they will provide.
     * \sa values() addValue()
     * \since Cutelyst 2.0.0
     */
    QVariant value(const QString &field) const;

    /*!
     * \brief Adds a new \a value extracted from the specified input \a field.
     * \param field Name of the input \link Request::parameters() parameter\endlink the value has been extracted from.
     * \param value Value as it has been extracted and maybe converted by the validator.
     * \sa values() value()
     * \since Cutelyst 2.0.0
     */
    void addValue(const QString &field, const QVariant &value);

    /*!
     * \brief Returns all extra data that has been extracted by the validators.
     * \return A QVariantHash where the key is the name of the input \link Request::parameters() parameter\endlink
     * and the value contains the extra data for that field.Have a look at the documentation of the specific
     * validators to see what kind of extra data they might generate.
     * \sa extra() addExtra()
     * \since Cutelyst 2.0.0
     */
    QVariantHash extras() const;

    /*!
     * \brief Returns the extra data for the input \a field.
     * \param field Name of the input \link Request::parameters() parameter\endlink to get the extra data for.
     * \return A QVariant containing extra data generated by the validators. If the \a field does not have any
     * extra data, a default constructed QVariant will be returned. Have a look at the documentation of the
     * specific validators to see what kind of extra data they might generate.
     * \sa extras() addExtra()
     * \since Cutelyst 2.0.0
     */
    QVariant extra(const QString &field) const;

    /*!
     * \brief Adds new \a extra data that came up when validating the input \a field.
     * \param field Name of the input \link Request::parameters() parameter\endlink the extra data occurred for.
     * \param extra The additional validation data.
     * \sa extras() extra()
     * \since Cutelyst 2.0.0
     */
    void addExtra(const QString &field, const QVariant &extra);

private:
    QSharedDataPointer<ValidatorResultPrivate> d;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORRESULT_H
