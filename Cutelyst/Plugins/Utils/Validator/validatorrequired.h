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
#ifndef CUTELYSTVALIDATORREQUIRED_H
#define CUTELYSTVALIDATORREQUIRED_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"

namespace Cutelyst {

class ValidatorRequiredPrivate;

/*!
 * \brief Checks if a field is available and not empty.
 *
 * The \a field under validation must be present in the input data and not empty.
 *
 * If ValidatorRule::trimBefore() is set to \c true (the default), whitespaces will be removed from
 * the beginning and the end of the input value before validation. So, fields that only contain whitespaces
 * will be treated as empty and are invalid.
 *
 * \link Validator See Validator for general usage of validators. \endlink
 *
 * \sa ValidatorRequiredIf, ValidatorRequiredUnless, ValidatorRequiredWith, ValidatorRequiredWithAll, ValidatorRequiredWithout, ValidatorRequiredWithoutAll
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorRequired : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new required validator.
     * \param field         Name of the input field to validate.
     * \param label         Human readable input field label, used for generic error messages.
     * \param customError   Custom error message if validation fails.
     */
    ValidatorRequired(const QString &field, const QString &label = QString(), const QString &customError = QString());

    /*!
     * \brief Deconstructs the required validator.
     */
    ~ValidatorRequired();

    /*!
     * \brief Performs the validation and returns an empty QString on success, otherwise an error message.
     */
    QString validate() const override;

protected:
    /*!
     * \brief Returns a generic error message.
     */
    QString genericValidationError() const override;

    /*!
     * Constructs a new ValidatorRequired object with the given private class.
     */
    ValidatorRequired(ValidatorRequiredPrivate &dd);

private:
    Q_DECLARE_PRIVATE(ValidatorRequired)
    Q_DISABLE_COPY(ValidatorRequired)
};

}

#endif //CUTELYSTVALIDATORREQUIRED_H
