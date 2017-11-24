/*
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef CUTELYSTVALIDATORRULE_H
#define CUTELYSTVALIDATORRULE_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/ParamsMultiMap>

#include <QScopedPointer>

namespace Cutelyst {

class ValidatorRulePrivate;

/*!
 * \brief Base class for all validators.
 *
 * This class can not be used on it's own, you have to create a derived class from it that implements your
 * validator logic. Or use one of the already existing derived classes.
 *
 * \par Writing a custom validator
 *
 * If you want to implement your own validator logic to use with Valiadtor, you have to create a class that
 * derives from ValidatorRule. The simplest implementation only needs a constructor an a reimplementation
 * of the validate() funciton. But for more comfort and usability, you should also reimplement validationError().
 * If your validator parses the input into a specific type to validate it and/or if you are using additional parameters,
 * you may also want to reimplement parsingError() and validationDataError() to return more appropriate generic error
 * messages.
 *
 * The most important parameter for every validator is the name of the \a field to validate. So your own validator should
 * require that field in the constructor. For better error messages you should also add an optional paramter to set
 * a \a label and maybe a \a customError message if validation fails.
 *
 * In the validation logic you have to return an empty QString if validation succeeded. Everything else will be treeted
 * as an error message and that the validation has been failed.
 *
 * So lets implement a custom validator that can check for a specific value to be set. (Maybe not a realistic example, but
 * it should be enough for demonstration.)
 *
 * \code{.cpp}
 * #include <Cutelyst/Plugins/Utils/ValidatorRule>
 *
 * class MyValidator : public Cutelyst::ValidatorRule
 * {
 * public:
 *     // field: name of the input field
 *     // compareValue: our custom value we want compare
 *     // label: an optional human readable label for generic error messages
 *     // customError: a custom error message that is shown if validation fails instead of genericValidationError()
 *     MyValidator::MyValidator(const QString &field, const QString &compareValue, const QString &label = QString(), const QString &customError = QString())
 *
 *     ~MyValidator();
 *
 *     // this will contain the validation logic
 *     // and should return an empty QString on success
 *     QString validate() const override;
 *
 * protected:
 *     // we want to have a generic error message
 *     QString validationError() const override;
 *
 * private:
 *     // storing our comparison value
 *     QString m_compareValue;
 * }
 *
 *
 * MyValidator::MyValidator(const QString &field, const QString &compareValue, const QString &label, const QString &customError) :
 *     Cutelyst::ValidatorRule(field, label, customError), m_compareValue(compareValue)
 * {
 * }
 *
 * MyValidator::~MyValidator()
 * {
 * }
 *
 * // when reimplementing the validate function, keep in mind, that
 * // an empty returned string is seen as successful validation, everything
 * // else will be seen as an error
 * QString MyValidator::validate() const
 * {
 *     QString result;
 *
 *     // lets get the field value
 *     const QString v = value();
 *
 *     // if our comparision value is empty, the validation should fail and we want
 *     // to return an error message according to this situation
 *     if (m_compareValue.isEmpty()) {
 *         result = validationDataError();
 *     } else {
 *
 *         // if the value is empty or the field is missing, the validation should succeed,
 *         // because we already have the required validators for that purpose
 *         // than we will compare our values and if they are not the same, we
 *         // will return an error string
 *         if (!v.isEmpty() && (m_compareValue != v)) {
 *              result = validationError();
 *         }
 *     }
 *
 *     // now let's return our result, if it is empty, validation was successfull
 *     return result;
 * }
 *
 *
 * QString MyValidator::genericValidationError() const
 * {
 *     QString error;
 *     // if no label is set, we will return a shorter error message
 *     if (label().isEmpty()) {
 *          error = tr("Must contain this value: %1).arg(m_compareValue);
 *     } else {
 *          error = tr("The %1 field must contain the following value: %2").arg(label(), m_compareValue);
 *     }
 *     return error;
 * }
 * \endcode
 *
 * That's it. Now you can use your own validator in the main Validator.
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorRule
{
public:
    /*!
     * \brief Constructs a new ValidatorRule with given parameters and \a parent.
     * \param field         Name of the field to validate.
     * \param label         Human readable input field label, used for generic error messages.
     * \param customError   Human readable custom error message if validation fails.
     */
    ValidatorRule(const QString &field, const QString &label = QString(), const QString &customError = QString());

    /*!
     * \brief Deconstructs the ValidatorRule.
     */
    ~ValidatorRule();

    /*!
     * \brief Returns an empty string on success, otherwise a string containing the error message.
     *
     * This is the main function to reimplement when writing a custom validator. When reimplementing
     * this function in a class derived from ValidatorRule, you have to return an empty string if
     * validation succeeded and the corresponding error if it fails. There are currently three error
     * functions that should be used for different error cases:
     *
     * \li validationError() - if validation itself fails
     * \li validationDataError() - if there is a problem with missing or invalid validation data, like comparison values
     * \li parsingError() - if the parsing of an input data fails in a valiator that not originally checks the parsing, but the parsing result
     *
     * \par Example
     *
     * \code{.cpp}
     * QString MyValidator::validate() const
     * {
     *      QString result;
     *
     *      if (m_myComparisonValue.isEmpty()) {
     *          result = validationDataError();
     *      } else {
     *          if (m_myComparisonValue != value()) {
     *              result = validationError();
     *          }
     *      }
     *
     *      return result;
     * }
     * \endcode
     */
    virtual QString validate() const = 0;

    /*!
     * \brief Returns the name of the field to validate.
     * \sa setField()
     */
    QString field() const;

    /*!
     * \brief Returns the human readable field label used for generic error messages.
     * \sa setLabel()
     */
    QString label() const;

    /*!
     * \brief Returns the field value.
     */
    QString value() const;

    /*!
     * \brief Returns true if field value should be trimmed before validation.
     *
     * By default, this will return \c true and all input values will be trimmed before validation to
     * remove whitespaces from the beginning and the end.
     *
     * \sa setTrimBefore()
     */
    bool trimBefore() const;



    /*!
     * \brief Sets the name of the field to validate.
     * \sa field()
     */
    void setField(const QString &field);

    /*!
     * \brief Sets human readable field label for generic error messages.
     * \sa label()
     */
    void setLabel(const QString &label);

    /*!
     * \brief Sets the request parameters to validate.
     * \sa parameters()
     */
    void setParameters(const ParamsMultiMap &params);

    /*!
     * \brief Returns the parameters to validate.
     * \sa setParameters()
     */
    ParamsMultiMap parameters() const;

    /*!
     * \brief Sets a cutom error returned with errorMessage()
     * \sa validationError()
     */
    void setCustomError(const QString &customError);

    /*!
     * \brief Sets a custom error message that is shown if parsing of input data fails.
     * \sa parsingError()
     */
    void setCustomParsingError(const QString &custom);

    /*!
     * \brief Sets a custom error message if validation data is invalid or missing.
     * \sa validationDataError()
     */
    void setCustomValidationDataError(const QString &custom);

    /*!
     * \brief Set to \c false to not trim input value before validation.
     *
     * By default, this value is set to \c true and all input values will be QString::trimmed() trimmed before validation to
     * remove whitespaces from the beginning and the end.
     *
     * \sa trimBefore()
     */
    void setTrimBefore(bool trimBefore);

protected:
    const QScopedPointer<ValidatorRulePrivate> d_ptr;
    /*!
     * Constructs a new ValidatorRule object with the given private class.
     */
    ValidatorRule(ValidatorRulePrivate &dd);

    /*!
     * \brief Returns a descriptive error message if validation failed.
     *
     * This will either return the custom error message set via setCustomError() or the message
     * returned by genericValidationError(). When writing a new ValidatorRule, use this in your
     * reimplementaion of validate() if validation failed.
     */
    QString validationError() const;

    /*!
     * \brief Returns a generic error mesage if validation failed.
     *
     * If you want to have a more specifc generic validation error message for your validator,
     * reimplment this in a subclass.
     */
    virtual QString genericValidationError() const;

    /*!
     * \brief Returns an error message if an error occured while parsing input.
     *
     * This will either return the custom parsing error message set via setCustomParsingError() or
     * the message returned by genericParsingError(). When writing a new ValidatorRule, use this
     * in your reimplementation of validate() if parsing of input data fails.
     */
    QString parsingError() const;

    /*!
     * \brief Returns a generic error message if an error occures while parsing input.
     *
     * If you want to have a more specific generic parsing error message for your validator,
     * reimplement this in a subclass.
     */
    virtual QString genericParsingError() const;

    /*!
     * \brief Returns an error message if any validation data is missing or invalid.
     *
     * Reimplement this in your sublcass to return an error message in case the validation data is missing
     * or invalid. The default implementation returns a generic message unless a custom one is set via setCustomValidationDataError().
     *
     * When reimplementing this function take into account that there might be a custom validation data error message set by the user.
     */
    QString validationDataError() const;

    /*!
     * \brief Returns a generic error message if any validation data is missing or invalid.
     *
     * If you want to have a more specific generic validation data error message for your validator,
     * reimplement this in a subclass.
     */
    virtual QString genericValidationDataError() const;

private:
    Q_DECLARE_PRIVATE(ValidatorRule)
    Q_DISABLE_COPY(ValidatorRule)
};

}

#endif //CUTELYSTVALIDATORRULE_H
