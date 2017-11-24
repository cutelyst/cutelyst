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
#ifndef CUTELYSTVALIDATORIN_H
#define CUTELYSTVALIDATORIN_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"

namespace Cutelyst {
    
class ValidatorInPrivate;

/*!
 * \brief Checks if the field value is one from a list of values.
 *
 * Validates if the \a field contains a value from the \a values list.
 *
 * If ValidatorRule::trimBefore() is set to \c true (the default), whitespaces will be removed from
 * the beginning and the end of the input value before validation. If the \a field's value is empty or if
 * the \a field is missing in the input data, the validation will succeed without performing the validation itself.
 * Use one of the \link ValidatorRequired required validators \endlink to require the field to be present and not empty.
 *
 * \link Validator See Validator for general usage of validators. \endlink
 *
 * \sa ValidatorNotIn
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorIn : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new in validator.
     * \param field         Name of the input field to validate.
     * \param values        List of values to compare against.
     * \param label         Human readable input field label, used for generic error messages.
     * \param customError   Custom error message if validation fails.
     */
    ValidatorIn(const QString &field, const QStringList &values, const QString &label = QString(), const QString &customError = QString());
    
    /*!
     * \brief Deconstructs the in validator.
     */
    ~ValidatorIn();
    
    /*!
     * \brief Performs the validation and returns an empty QString on success, otherwise an error message.
     */
    QString validate() const override;

    /*!
     * \brief Sets the values to compare against.
     */
    void setValues(const QStringList &values);
    
protected:
    /*!
     * \brief Returns a generic error message.
     */
    QString genericValidationError() const override;
    
    /*!
     * Constructs a new ValidatorIn object with the given private class.
     */
    ValidatorIn(ValidatorInPrivate &dd);
    
private:
    Q_DECLARE_PRIVATE(ValidatorIn)
    Q_DISABLE_COPY(ValidatorIn)
};
    
}

#endif //CUTELYSTVALIDATORIN_H

