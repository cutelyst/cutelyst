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
#ifndef CUTELYSTVALIDATORAFTER_H
#define CUTELYSTVALIDATORAFTER_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"

namespace Cutelyst {

class ValidatorAfterPrivate;

/*!
 * \brief Checks if a date, time or datetime is after a comparison value.
 *
 * This will check if the date, time or datetime in the input \a field is after a comparison value set via \a dateTime in the constructor
 * or via setDateTime(). It depends on the comparison value how the input data is handled. If the comparative value is a QDateTime,
 * the input data will be converted into a QDateTime to compare the both values and so on.
 *
 * If the input data can not be parsed into a comparable format, the validation fails and ValidatorRule::parsingError() will return \c true.
 * It will also fail if the comparative value is not of type QDate, QTime or QDateTime. Use \a inputFormat parameter of the constructor
 * or setInputFormat() to set a custom input format. If no input format has been set, the validator will try to parse the input according
 * to the following definitions: Qt::ISODate, Qt::RFC2822Date, Qt::TextDate
 *
 * If ValidatorRule::trimBefore() is set to \c true (the default), whitespaces will be removed from
 * the beginning and the end of the input value before validation. If the \a field's value is empty or if
 * the \a field is missing in the input data, the validation will succeed without performing the validation itself.
 * Use one of the \link ValidatorRequired required validators \endlink to require the field to be present and not empty.
 *
 * \link Validator See Validator for general usage of validators. \endlink
 *
 * \sa ValidatorBefore
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorAfter : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new after validator.
     * \param field         Name of the input field to validate.
     * \param dateTime      QDate, QTime or QDateTime to compare against. Any other type will lead to a validation data error and valiation fails.
     * \param inputFormat   Optional input format for input data parsing.
     * \param label         Human readable input field label, used for generic error messages.
     * \param customError   Custom error message if validation fails.
     */
    ValidatorAfter(const QString &field, const QVariant &dateTime, const QString &inputFormat = QString(), const QString &label = QString(), const QString &customError = QString());

    /*!
     * \brief Deconstructs the after validator.
     */
    ~ValidatorAfter();

    /*!
     * \brief Performs the validation and returns an empty QString on success, otherwise an error message.
     */
    QString validate() const override;

    /*!
     * \brief Sets the QDate, QTime or QDateTime to compare against.
     */
    void setDateTime(const QVariant &dateTime);

    /*!
     * \brief Sets optional format for input data parsing.
     */
    void setInputFormat(const QString &format);

protected:
    QString genericValidationError() const override;

    ValidatorAfter(ValidatorAfterPrivate &dd);

private:
    Q_DECLARE_PRIVATE(ValidatorAfter)
    Q_DISABLE_COPY(ValidatorAfter)
};


}

#endif //CUTELYSTVALIDATORAFTER_H
