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
#ifndef CUTELYSTVALIDATORTIME_H
#define CUTELYSTVALIDATORTIME_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"

namespace Cutelyst {
    
class ValidatorTimePrivate;

/*!
 * \brief Checks if the input data is a valid time.
 *
 * This validator checks if the input \a field can be parsed into a QTime, it will check the parsing ability but will not convert the
 * input data into a QTime. If a custom \a format is given, the validator will at first try to parse the date according to that \a format.
 * If that fails, it will try to parse the date based on standard formats in the following order: Qt::ISODate, Qt::RFC2822Date, Qt::TextDate
 *
 * If ValidatorRule::trimBefore() is set to \c true (the default), whitespaces will be removed from
 * the beginning and the end of the input value before validation. If the \a field's value is empty or if
 * the \a field is missing in the input data, the validation will succeed without performing the validation itself.
 * Use one of the \link ValidatorRequired required validators \endlink to require the field to be present and not empty.
 *
 * \link Validator See Validator for general usage of validators. \endlink
 *
 * \sa ValidatorDateTime, ValidatorDate
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorTime : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new time validator.
     * \param field         Name of the input field to validate.
     * \param format        Optional time format for input parsing.
     * \param label         Human readable input field label, used for generic error messages.
     * \param customError   Custom error message if validation fails.
     */
    ValidatorTime(const QString &field, const QString &format = QString(), const QString &label = QString(), const QString &customError = QString());
    
    /*!
     * \brief Deconstructs time the validator.
     */
    ~ValidatorTime();
    
    /*!
     * \brief Performs the validation.
     *
     * Returns \c true on success.
     */
    bool validate() override;

    /*!
     * \brief Sets an optional date format.
     */
    void setFormat(const QString &format);
    
protected:
    /*!
     * \brief Returns a generic error message.
     */
    QString genericErrorMessage() const override;
    
    ValidatorTime(ValidatorTimePrivate &dd);
    
private:
    Q_DECLARE_PRIVATE(ValidatorTime)
    Q_DISABLE_COPY(ValidatorTime)
};
    
}

#endif //CUTELYSTVALIDATORTIME_H

