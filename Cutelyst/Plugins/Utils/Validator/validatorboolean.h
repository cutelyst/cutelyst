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
#ifndef CUTELYSTVALIDATORBOOLEAN_H
#define CUTELYSTVALIDATORBOOLEAN_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"

namespace Cutelyst {
    
class ValidatorBooleanPrivate;

/*!
 * \brief Checks if a value can be casted into a boolean.
 *
 * The \a field under validation must constain one of the following acceptable input values: \c 1, \c 0, \c true, \c false, \c on and \c off.
 * This will only check for this values, but will not convert the input into a boolean.
 *
 * If ValidatorRule::trimBefore() is set to \c true (the default), whitespaces will be removed from
 * the beginning and the end of the input value before validation. If the \a field's value is empty or if
 * the \a field is missing in the input data, the validation will succeed without performing the validation itself.
 * Use one of the \link ValidatorRequired required validators \endlink to require the field to be present and not empty.
 *
 * \link Validator See Validator for general usage of validators. \endlink
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorBoolean : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new validator.
     * \param field         Name of the input field to validate.
     * \param label         Human readable input field label, used for generic error messages.
     * \param customError   Custom error message if validation fails.
     */
    ValidatorBoolean(const QString &field, const QString &label = QString(), const QString &customError = QString());
    
    /*!
     * \brief Deconstructs the validator.
     */
    ~ValidatorBoolean();
    
    /*!
     * \brief Performs the validation.
     *
     * Returns \c true on success.
     */
    bool validate() override;
    
protected:
    /*!
     * \brief Returns a generic error message.
     */
    QString genericErrorMessage() const override;
    
    ValidatorBoolean(ValidatorBooleanPrivate &dd);
    
private:
    Q_DECLARE_PRIVATE(ValidatorBoolean)
    Q_DISABLE_COPY(ValidatorBoolean)
};
    
}

#endif //CUTELYSTVALIDATORBOOLEAN_H

