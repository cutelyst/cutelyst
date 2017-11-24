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
#ifndef CUTELYSTVALIDATORALPHADASH_H
#define CUTELYSTVALIDATORALPHADASH_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"

namespace Cutelyst {

class ValidatorAlphaDashPrivate;

/*!
 * \brief Checks a value for only alpha-numeric content and dashes and underscores.
 *
 * The \a field under validation is only allowed to contain alpha-numeric characters as well as dashes and underscores.
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
 * "Hallo_Köddel-2" // valid
 * " " // valid if trimBefore is true, invalid if trimBefore is false
 * \endcode
 *
 * \sa ValidatorAlpha, ValidatorAlphaNum
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorAlphaDash : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new alpha dash validator.
     * \param field         Name of the input field to validate.
     * \param label         Human readable input field label, used for generic error messages.
     * \param customError   Custom error message if validation fails.
     */
    ValidatorAlphaDash(const QString &field, const QString &label = QString(), const QString &customError = QString());

    /*!
     * \brief Deconstructs the alpha dash validator.
     */
    ~ValidatorAlphaDash();

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
     * Constructs a new ValidatorAlphaDash object with the given private class.
     */
    ValidatorAlphaDash(ValidatorAlphaDashPrivate &dd);

private:
    Q_DECLARE_PRIVATE(ValidatorAlphaDash)
    Q_DISABLE_COPY(ValidatorAlphaDash)
};

}

#endif //CUTELYSTVALIDATORALPHADASH_H
