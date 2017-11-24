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
#ifndef CUTELYSTVALIDATORALPHANUM_H
#define CUTELYSTVALIDATORALPHANUM_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"

namespace Cutelyst {

class ValidatorAlphaNumPrivate;

/*!
 * \brief Checks a value for only alpha-numeric content.
 *
 * The \a field under validation is only allowed to contain alpha-numeric characters.
 *
 * If ValidatorRule::trimBefore() is set to \c true (the default), whitespaces will be removed from
 * the beginning and the end of the input value before validation. If the \a field's value is empty or if
 * the \a field is missing in the input data, the validation will succeed without performing the validation itself.
 * Use one of the \link ValidatorRequired required validators \endlink to require the field to be present and not empty.
 *
 * \link Validator See Validator for general usage of validators. \endlink
 *
 * \par Examples
 * \code
 * "Hallo Kuddel!" // invalid
 * "HalloKöddel2" // valid
 * " " // valid if trimBefore is true, invaid if trimBefore is false
 * \endcode
 *
 * \sa ValidatorAlpha, ValidatorAlphaDash
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorAlphaNum : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new alpha num validator.
     * \param field         Name of the input field to validate.
     * \param label         Human readable input field label, used for error messages.
     * \param customError   Custom errror message if validation fails.
     */
    ValidatorAlphaNum(const QString &field, const QString &label = QString(), const QString &customError = QString());

    /*!
     * \brief Deconstructs the alpha num validator.
     */
    ~ValidatorAlphaNum();

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
     * Constructs a new ValidatorAlphaNum object with the given private class.
     */
    ValidatorAlphaNum(ValidatorAlphaNumPrivate &dd);

private:
    Q_DECLARE_PRIVATE(ValidatorAlphaNum)
    Q_DISABLE_COPY(ValidatorAlphaNum)
};

}

#endif //CUTELYSTVALIDATORALPHANUM_H
