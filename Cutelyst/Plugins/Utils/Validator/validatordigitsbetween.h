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
#ifndef CUTELYSTVALIDATORDIGITSBETWEEN_H
#define CUTELYSTVALIDATORDIGITSBETWEEN_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"

namespace Cutelyst {
    
class ValidatorDigitsBetweenPrivate;

/*!
 * \brief Checks for digits only with a length between min and max.
 *
 * The field under validation must only contain digits with a length between \a min and \a max. The digits
 * are not interpreteded as a numeric value but as a string, so the \a min and \a max values are not a range
 * for a numeric value but for the string length. Both values default to \c 0 what will disable the range check.
 *
 * If ValidatorRule::trimBefore() is set to \c true (the default), whitespaces will be removed from
 * the beginning and the end of the input value before validation. If the \a field's value is empty or if
 * the \a field is missing in the input data, the validation will succeed without performing the validation itself.
 * Use one of the \link ValidatorRequired required validators \endlink to require the field to be present and not empty.
 *
 * \link Validator See Validator for general usage of validators. \endlink
 *
 * \sa ValidatorDigits
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorDigitsBetween : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new digits between validator.
     * \param field         Name of the input field to validate.
     * \param min           Minimum length of the digits
     * \param max           Maximum length of the digits
     * \param label         Human readable input field label, used for generic error messages.
     * \param customError   Custom error message if validation fails.
     */
    ValidatorDigitsBetween(const QString &field, int min, int max, const QString &label = QString(), const QString &customError = QString());
    
    /*!
     * \brief Deconstructs the digits between validator.
     */
    ~ValidatorDigitsBetween();
    
    /*!
     * \brief Performs the validation and returns an empty QString on success, otherwise an error message.
     */
    QString validate() const override;

    /*!
     * \brief Sets the minimum length.
     */
    void setMin(int min);

    /*!
     * \brief Sets the maximum length.
     */
    void setMax(int max);
    
protected:
    /*!
     * \brief Returns a generic error message.
     */
    QString genericValidationError() const override;
    
    /*!
     * Constructs a new ValidatorDigitsBetween object with the given private class.
     */
    ValidatorDigitsBetween(ValidatorDigitsBetweenPrivate &dd);
    
private:
    Q_DECLARE_PRIVATE(ValidatorDigitsBetween)
    Q_DISABLE_COPY(ValidatorDigitsBetween)
};
    
}

#endif //CUTELYSTVALIDATORDIGITSBETWEEN_H

