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
#include <QObject>

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
 * #include <QObject>
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
 *         setValid(true);
 *         return true;
 *     }
 *
 *     // if our comparision value is empty, the validation should fail and we will
 *     // set setValidationDataError() to true to return the appropriate error message
 *     if (m_compareValue.isEmpty()) {
 *         setValidationDataError(true);
 *         return false;
 *     }
 *
 *     // now lets compare our values
 *     if (m_compareValue == value()) {
 *         setValid(true);
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
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorRule : public QObject
{
    Q_OBJECT
    /*!
     * \brief Name of the field to validate.
     *
     * \par Access functions:
     * <TABLE><TR><TD>QString</TD><TD>field() const</TD></TR><TR><TD>void</TD><TD>setField(const QString &nField)</TD></TR></TABLE>
     */
    Q_PROPERTY(QString field READ field WRITE setField)
    /*!
     * \brief Human readable label of the field.
     *
     * Will be used to create a generic error message if no \link ValidatorRule::customError custom error \endlink is set. If label
     * is empty, the name of the \link ValidatorRule::field field \endlink will be used.
     *
     * \par Access functions:
     * <TABLE><TR><TD>QString</TD><TD>label() const</TD></TR><TR><TD>void</TD><TD>setLabel(const QString &nLabel)</TD></TR></TABLE>
     */
    Q_PROPERTY(QString label READ label WRITE setLabel)
    /*!
     * \brief Value that should be validated.
     *
     * This is extracted from the parameters set via setParameters() together with the \link ValidatorRule::field field \endlink property.
     *
     * \par Access functions:
     * <TABLE><TR><TD>QString</TD><TD>value() const</TD></TR></TABLE>
     */
    Q_PROPERTY(QString value READ value)
    /*!
     * \brief Custom error message that should be shown if validation fails.
     *
     * If no custom error message is set, \link ValidatorRule::errorMessage errorMessage \endlink will return a generic error message.
     *
     * \par Access functions:
     * <TABLE><TR><TD>QString</TD><TD>customError() const</TD></TR><TR><TD>void</TD><TD>setCustomError(const QString &nCustomError)</TD></TR></TABLE>
     */
    Q_PROPERTY(QString customError READ customError WRITE setCustomError)
    /*!
     * \brief Returns \c true if the validation was successfull.
     *
     * Defaults to \c false.
     *
     * \par Access functions:
     * <TABLE><TR><TD>bool</TD><TD>isValid() const</TD></TR></TABLE>
     */
    Q_PROPERTY(bool valid READ isValid)
    /*!
     * \brief Error message returned if the validation fails.
     *
     * This contains a human readable error message if the validation fails. If \link ValidatorRule::customError customError \endlink is set, it will be
     * returned, otherwise a generic error message will be created that uses the \link ValidatorRule::label label \endlink or the \link ValidatorRule::field field \endlink property.
     *
     * \par Access functions:
     * <TABLE><TR><TD>QString</TD><TD>errorMessage() const</TD></TR></TABLE>
     */
    Q_PROPERTY(QString errorMessage READ errorMessage)
    /*!
     * \brief True if input data parsing fails.
     *
     * \par Access functions:
     * <TABLE><TR><TD>bool</TD><TD>parsingError() const</TD></TR></TABLE>
     */
    Q_PROPERTY(bool parsingError READ parsingError)
    /*!
     * \brief True if the validation data is missing or unusable.
     *
     * \par Access functions:
     * <TABLE><TR><TD>bool</TD><TD>validationDataError() const</TD></TR></TABLE>
     */
    Q_PROPERTY(bool validationDataError READ validationDataError)
public:
    /*!
     * \brief Constructs a new ValidatorRule with given parameters and \a parent.
     * \param field         Name of the field to validate.
     * \param label         Human readable input field label, used for generic error messages.
     * \param customError   Human readable custom error message if validation fails.
     * \param parent        Parent object.
     */
    ValidatorRule(const QString &field, const QString &label = QString(), const QString &customError = QString(), QObject *parent = nullptr);

    /*!
     * \brief Deconstructs the ValidatorRule.
     */
    ~ValidatorRule();

    /*!
     * \brief Reimplement this in a subclass to perform the validation.
     *
     * When reimplementing this function, do not forget to set the validity with setValid().
     *
     * \par Example
     *
     * \code{.cpp}
     * bool MyValidator::validate() const
     * {
     *      if (value().isEmpty()) {
     *          return false;
     *      } else {
     *          setValid(true);
     *          return true;
     *      }
     * }
     * \endcode
     */
    virtual bool validate() = 0;

    /*!
     * \brief Getter function for the \link ValidatorRule::field field \endlink property.
     * \sa ValidatorRule::setField()
     */
    QString field() const;
    /*!
     * \brief Getter function for the \link ValidatorRule::label label \endlink property.
     * \sa ValidatorRule::setLabel()
     */
    QString label() const;
    /*!
     * \brief Getter function for the \link ValidatorRule::value value \endlink property.
     * \sa ValidatorRule::setValue()
     */
    QString value() const;
    /*!
     * \brief Getter function for the \link ValidatorRule::customError customError \endlink property.
     * \sa ValidatorRule::setCustomError()
     */
    QString customError() const;
    /*!
     * \brief Getter function for the \link ValidatorRule::valid valid \endlink property.
     * \sa ValidatorRule::setValid()
     */
    bool isValid() const;
    /*!
     * \brief Getter function for the \link ValidatorRule::errorMessage errorMessage \endlink property.
     * \sa ValidatorRule::setErrorMessage()
     */
    QString errorMessage() const;
    /*!
     * \brief Returns true if input data parsing fails.
     *
     * \sa setParsingError(), ValidatorRule::parsingError
     */
    bool parsingError() const;
    /*!
     * \brief Returns true if the validation data is missing or unusable.
     *
     * \sa setValidationDataError(), ValidatorRule::validationDataError
     */
    bool validationDataError() const;
    /*!
     * \brief Returns true if field value should be trimmed before validation.
     *
     * Default is \c true.
     *
     * \sa setTrimBefore()
     */
    bool trimBefore() const;



    /*!
     * \brief Setter function for the \link ValidatorRule::field field \endlink property.
     * \sa ValidatorRule::field()
     */
    void setField(const QString &field);
    /*!
     * \brief Setter function for the \link ValidatorRule::label label \endlink property.
     * \sa ValidatorRule::label()
     */
    void setLabel(const QString &label);
    /*!
     * \brief Setter function for the \link ValidatorRule::customError customError \endlink property.
     * \sa ValidatorRule::customError()
     */
    void setCustomError(const QString &customError);

    /*!
     * \brief Sets the request parameters to validate.
     *
     * \sa parameters()
     */
    void setParameters(const ParamsMultiMap &params);

    /*!
     * \brief Returns the parameters to validate.
     *
     * \sa setParameters()
     */
    ParamsMultiMap parameters() const;

    /*!
     * \brief Sets a custom error message that is shown if parsing of input data fails.
     *
     * \sa customParsingError(), parsingErrorMessage()
     */
    void setCustomParsingError(const QString &custom);

    /*!
     * \brief Sets a custom error message if validation data is invalid or missing.
     *
     * \sa customValidationDataError(), validationDataErrorMessage()
     */
    void setCustomValidationDataError(const QString &custom);

    /*!
     * \brief Set to \c false to not trim input value before validation.
     *
     * Default is \c true.
     */
    void setTrimBefore(bool trimBefore);

protected:
    const QScopedPointer<ValidatorRulePrivate> d_ptr;
    ValidatorRule(ValidatorRulePrivate &dd, QObject *parent);

    /*!
     * \brief Returns a generic error message.
     *
     * Reimplement this in your subclass to return a generic error message that will be used by the \link ValidatorRule::errorMessage errorMessage \endlink property.
     *
     * The default implementation returns a (translated) string like this: "The input data in the “Foo Bar” field is not valid.".
     *
     * When reimplementing this, you should at first look if the \link ValidatorRule::label label \endlink property is set
     * and use it in your generic message. If that is empty, use the \link ValidatorRule::field field \endlink property.
     *
     * \par Example
     *
     * \code{.cpp}
     * QString MyValidator::genericErrorMessage() const
     * {
     *      QString fn = !label().isEmpty() ? label() : field();
     *      return tr("The “%1” field has the wrong content.").arg(fn);
     * }
     * \endcode
     */
    virtual QString genericErrorMessage() const;

    /*!
     * \brief Returns an error message if an error occured while parsing input.
     *
     * Reimplement this in your subclass to return an error message that will be returned if the input data could not be parsed
     * into a comparative value. The default implementation returns a generic message unless a custom one is set via setCustomParsingError().
     *
     * When reimplementing this function take into account that there might be a custom parsing error message set by the user.
     */
    virtual QString parsingErrorMessage() const;


    /*!
     * \brief Returns an error messag if an validation data is missing or invalid.
     *
     * Reimplement this in your sublcass to return an error message that will be returned if the validation data is missing
     * or invalid. The default implementation return a generic message unless a custom one is set via setCustomValidationDataError().
     *
     * When reimplementing this function take into account taht there might be a cstom parsing error message set by the user.
     */
    virtual QString validationDataErrorMessage() const;

    /*!
     * \brief Returns the name of the field for the generic error message.
     *
     * This can be used by genericErrorMessage() to retrieve the field name.
     * It will return the \link ValidatorRule::label label \endlink property if it is set, otherwise
     * it will return the \link ValidatorRule::field field \endlink property.
     */
    QString genericFieldName() const;

    /*!
     * \brief Set this in your subclass to true if validation was successful.
     */
    void setValid(bool valid);

    /*!
     * \brief Set this to true if parsing of input data fails.
     *
     * Set \a parsingError to \c true if the parsing of input data fails for further checks. If you want to check
     * a date range for example, you at first want to parse the input data into a date (QDate), if that parsing
     * fails, set \a parsingError to \c true. Setting this to \c true will automatically set setValid() to \c false.
     *
     * This information will be used to provide a valid error message.
     *
     * \sa parsingError()
     */
    void setParsingError(bool parsingError);

    /*!
     * \brief Set this to true if validation data is missing or unusable.
     *
     * Set \a validationDataError to \c true if the data to validate the input against is missing or unusable. If
     * you want to check a date or time range for example, you expect a QDate, QDateTime or QDate to check against
     * but if there is something else or nothing, set \a validationDataError to \c true.
     * Setting this to \c true will automatically set setValid() to \c false.
     *
     * This information will be used to provide a valid error message.
     *
     * \sa validationDataError()
     */
    void setValidationDataError(bool validationDataError);

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

private:
    Q_DECLARE_PRIVATE(ValidatorRule)
    Q_DISABLE_COPY(ValidatorRule)
};

}

#endif //CUTELYSTVALIDATORRULE_H
