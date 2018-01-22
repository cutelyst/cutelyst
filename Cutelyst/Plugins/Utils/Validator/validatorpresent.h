/*
 * Copyright (C) 2017-2018 Matthias Fehring <kontakt@buschmann23.de>
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
#ifndef CUTELYSTVALIDATORPRESENT_H
#define CUTELYSTVALIDATORPRESENT_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"

namespace Cutelyst {
    
class ValidatorPresentPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \brief The field under validation must be present in input data but can be empty.
 *
 * This validator checks if the \a field is present in the input data, but not if it contains any content.
 * If you want to check the fields presence and require it to have content, use one of the \link ValidatorRequired required validators \endlink.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorFilled, ValidatorRequired
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorPresent : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new present validator.
     * \param field         Name of the input field to validate.
     * \param messages      Custom error message if validation fails.
     */
    ValidatorPresent(const QString &field, const ValidatorMessages &messages = ValidatorMessages());
    
    /*!
     * \brief Deconstructs the present validator.
     */
    ~ValidatorPresent();
        
protected:
    /*!
     * \brief Performs the validation and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input paramter
     * value as QString.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /*!
     * \brief Returns a generic error message if validation failed.
     */
    QString genericValidationError(Context *c, const QVariant &errorData = QVariant()) const override;
    
private:
    Q_DECLARE_PRIVATE(ValidatorPresent)
    Q_DISABLE_COPY(ValidatorPresent)
};
    
}

#endif //CUTELYSTVALIDATORPRESENT_H

