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
 * If you want to implement your own validator logic to use with Valiator, you have to create a class that
 * derives from ValidatorRule. The simplest implementation only needs a constructor an a reimplementation
 * of the validate() funciton. But for more comfort and usability, you should also reimplement genericErrorMessage().
 * If your validator parses the input into a specific type to validate it and/or if you are using additional parameters,
 * you may also want to reimplement parsingErrorMessage() and validationDataErrorMessage().
 *
 * The most important parameter for every validator is the name of the field to validate. So your own validator should
 * require that field in the constructor. For better error messages you should also add an optional paramter to set
 * a label and maybe a custom error message.
 *
 * In the validation logic you have to set setValid() to \c true if validation succeedes (it is set to \c false by default)
 * and return \c true.
 *
 * So lets implement a custom validator that can check for a specific value to be set. (Maybe not a realistic example, but
 * it should be enough for demonstration.)
 *
 * \code{.cpp}
 * #include <Cutelyst/Plugins/Utils/ValidatorRule>
 *
 * class MyValidator : public Cutelyst::ValidatorRule
 * {
 *     Q_OBJECT
 * public:
 *     // field: name of the input field
 *     // compareValue: our custom value we want compare
 *     // label: an optional human readable label for the generic error message
 *     // customError: a custom error message that is shown if validation fails
 *     // parent: you should add a parent, so that Validator can control the destruction
 *     MyValidator::MyValidator(const QString &field, const QString &compareValue, const QString &label = QString(), const QString &customError = QString(), QObject *parent = nullptr)
 *
 *     ~MyValidator();
 *
 *     // this will contain the validation logic
 *     bool validate() override;
 *
 * protected:
 *     // we want to have a generic error message
 *     QString genericErrorMessage() const override;
 *
 * private:
 *     // storing our comparison value
 *     QString m_compareValue;
 * }
 *
 *
 * MyValidator::MyValidator(const QString &field, const QString &compareValue, const QString &label, const QString &customError, QObject *parent) :
 *     Cutelyst::ValidatorRule(field, label, customError, parent)
 * {
 *     m_compareValue;
 * }
 *
 * MyValidator::~MyValidator()
 * {
 * }
 *
 * bool MyValidator::validate()
 * {
 *     // lets get the field value
 *     QString v = value();
 *
 *     // if the value is empty or the field is missing, the validation should succeed,
 *     // because we already have the required validators for that purpose
 *     if (v.isEmpts()) {
 *         setError(ValidatorRule::NoError);
 *         return true;
 *     }
 *
 *     // if our comparision value is empty, the validation should fail and we will
 *     // set setValidationDataError() to true to return the appropriate error message
 *     if (m_compareValue.isEmpty()) {
 *         setError(ValidatorRule::ValidationDataError);
 *         return false;
 *     }
 *
 *     // now lets compare our values
 *     if (m_compareValue == value()) {
 *         setError(ValidatorRule::NoError);
 *         return true;
 *     }
 *
 *     return false;
 * }
 *
 *
 * QString MyValidator::genericErrorMessage() const
 * {
 *     return tr("The “%1” field must constain the following value: %2").arg(genericFieldName(), m_compareValue);
 * }
 * \endcode
 *
 * That's it. Now you can use your own validator in the main Validator.
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorRule
{
public:
    /*!
     * \brief Defines the validation status.
     */
    enum ValidatonErrorType {
        NoError             = 0,    /**< The validation succeeded and the input data is valid. */
        ValidationFailed    = 1,    /**< Validation failed and data is not valid. */
        ParsingError        = 2,    /**< The input data could not be parsed into a comparable type. Validation failed. */
        ValidationDataError = 3     /**< The data to validate against is missing or unusable. Validation failed. */
    };

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
     * \brief Reimplement this in a subclass to perform the validation.
     *
     * When reimplementing this function, do not forget to set the status via setError().
     *
     * \par Example
     *
     * \code{.cpp}
     * bool MyValidator::validate() const
     * {
     *      if (m_myComparisonValue.isEmpty()) {
     *          setError(ValidatorRule::ValidationDataError);
     *          return false;
     *      }
     *
     *      if (m_myComparisonValue == value()) {
     *          setError(ValidatorRule::NoError)
     *          return true;
     *      } else {
     *          return false;
     *      }
     * }
     * \endcode
     */
    virtual bool validate() = 0;

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
     * \brief Returns the custom error.
     * \sa setCustomError()
     */
    QString customError() const;

    /*!
     * \brief Returns \c true if the validation was successful.
     *
     * Returns \c true if the validation was successful and error() returns ValidatorRule::NoError.
     */
    bool isValid() const;

    /*!
     * \brief Returns an error message if validation fails.
     *
     * Use isValid() or error() to check if the validation failed. Depending on the ValidatorRule::ValidationErrorType it
     * will return the appropriate error message, either a generic one, or if set, a custom one.
     *
     * \sa setCustomError(), setCustomParsingError(), setCustomValidationDataError()
     */
    QString errorMessage() const;

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
     * \brief Returns the error type.
     *
     * By default this will return ValidationRule::ValidationFailed. Only if validation
     * has been performed and was successful, it will return ValidationRule::NoError.
     *
     * If this returns ValidationRule::NoError, isValid() will return \c true.
     */
    ValidatonErrorType error() const;



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
     * \brief Sets a cutom error returned with errorMessage()
     * \sa customError()
     */
    void setCustomError(const QString &customError);

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
     * \brief Sets a custom error message that is shown if parsing of input data fails.
     * \sa customParsingError(), parsingErrorMessage()
     */
    void setCustomParsingError(const QString &custom);

    /*!
     * \brief Sets a custom error message if validation data is invalid or missing.
     * \sa customValidationDataError(), validationDataErrorMessage()
     */
    void setCustomValidationDataError(const QString &custom);

    /*!
     * \brief Set to \c false to not trim input value before validation.
     *
     * By default, this value is set to \c true and all input values will be trimmed before validation to
     * remove whitespaces from the beginning and the end.
     *
     * \sa trimBefore()
     */
    void setTrimBefore(bool trimBefore);

protected:
    const QScopedPointer<ValidatorRulePrivate> d_ptr;
    ValidatorRule(ValidatorRulePrivate &dd);

    /*!
     * \brief Returns a generic error message.
     *
     * Reimplement this in your subclass to return a generic error message that will be used by the ValidatorRule::errorMessage() function.
     *
     * The default implementation returns a string like this: "The input data in the “Foo Bar” field is not valid.".
     *
     * When reimplementing this, you should the genericFieldName() function to either get a label, if any has been set, or the field name.
     *
     * \par Example
     *
     * \code{.cpp}
     * QString MyValidator::genericErrorMessage() const
     * {
     *      return tr("The “%1” field has the wrong content.").arg(genericFieldName());
     * }
     * \endcode
     */
    virtual QString genericErrorMessage() const;

    /*!
     * \brief Returns an error message if an error occured while parsing input.
     *
     * Reimplement this in your subclass to return an error message in case the input data could not be parsed
     * into a comparative value. The default implementation returns a generic message unless a custom one is set via setCustomParsingError().
     *
     * When reimplementing this function take into account that there might be a custom parsing error message set by the user.
     */
    virtual QString parsingErrorMessage() const;

    /*!
     * \brief Returns an error message if any validation data is missing or invalid.
     *
     * Reimplement this in your sublcass to return an error message in case the validation data is missing
     * or invalid. The default implementation returns a generic message unless a custom one is set via setCustomValidationDataError().
     *
     * When reimplementing this function take into account that there might be a custom validation data error message set by the user.
     */
    virtual QString validationDataErrorMessage() const;

    /*!
     * \brief Returns the name of the field for the generic error message.
     *
     * This can be used by genericErrorMessage() to retrieve the field name.
     * It will return the \link ValidatorRule::label() label() \endlink if it is set, otherwise
     * it will return the \link ValidatorRule::field() field() \endlink.
     */
    QString genericFieldName() const;

    /*!
     * \brief Returns the custom parsing error message if any is set.
     *
     * \sa setCustomParsingError(), parsingErrorMessage()
     */
    QString customParsingError() const;

    /*!
     * \brief Returns the custom validation data error message if any is set.
     *
     * \sa setCustomValidationDataError(), validationDataErrorMessage()
     */
    QString customValidationDataError() const;

    /*!
     * \brief Sets the error type.
     *
     * Use this to set the result of the validation when reimplementing ValidatorRule. The default value is ValidationRule::ValidationError,
     * if the validation succeeded, set this to ValidationRule::NoError.
     */
    void setError(ValidatonErrorType errorType);

private:
    Q_DECLARE_PRIVATE(ValidatorRule)
    Q_DISABLE_COPY(ValidatorRule)
};

}

#endif //CUTELYSTVALIDATORRULE_H
