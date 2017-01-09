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
#ifndef CUTELYSTVALIDATORDIGITS_H
#define CUTELYSTVALIDATORDIGITS_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"
#include <QObject>

namespace Cutelyst {
    
class ValidatorDigitsPrivate;

/*!
 * \brief Checks for digits only with optional length check.
 *
 * The \a field under validation must only contain digits with an optional exact \a length. If length is set to a value lower
 * or equal to \c 0, the length check will not be performed. The input digits will not be interpreted as numeric values
 * but as a string. So the length is not meant to test for an exact numeric value but for the string length.
 *
 * If ValidatorRule::trimBefore() is set to \c true (the default), whitespaces will be removed from
 * the beginning and the end of the input value before validation. If the \a field's value is empty or if
 * the \a field is missing in the input data, the validation will succeed without performing the validation itself.
 * Use one of the \link ValidatorRequired required validators \endlink to require the field to be present and not empty.
 *
 * \link Validator See Validator for general usage of validators. \endlink
 *
 * \sa ValidatorDigitsBetween
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorDigits : public ValidatorRule
{
    Q_OBJECT
public:
    /*!
     * \brief Constructs a new digits validator.
     * \param field         Name of the input field to validate.
     * \param length        Length of the digits, defaults to \c -1 what disables the check.
     * \param label         Human readable input field label, used for generic error messages.
     * \param customError   Custom error message if validation fails.
     * \param parent        Parent object.
     */
    ValidatorDigits(const QString &field, int length = -1, const QString &label = QString(), const QString &customError = QString(), QObject *parent = nullptr);
    
    /*!
     * \brief Deconstructs the digits validator.
     */
    ~ValidatorDigits();
    
    /*!
     * \brief Performs the validation.
     *
     * Returns \c true on success.
     */
    bool validate() override;

    /*!
     * \brief Sets the allowed length of the input data.
     *
     * Defaults to \c -1 what disables the length check.
     */
    void setLength(int length);

protected:
    /*!
     * \brief Returns a generic error message.
     */
    QString genericErrorMessage() const override;
    
    ValidatorDigits(ValidatorDigitsPrivate &dd, QObject *parent);
    
private:
    Q_DECLARE_PRIVATE(ValidatorDigits)
    Q_DISABLE_COPY(ValidatorDigits)
};
    
}

#endif //CUTELYSTVALIDATORDIGITS_H

