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
#ifndef CUTELYSTVALIDATORREQUIRED_H
#define CUTELYSTVALIDATORREQUIRED_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"

namespace Cutelyst {

class ValidatorRequiredPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorRequired validatorrequired.h <Cutelyst/Plugins/Utils/validatorrequired.h>
 * \brief Checks if a field is available and not empty.
 *
 * The \a field under validation must be present in the input data and not empty.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation.
 * So, fields that only contain whitespaces will be treated as empty and are invalid.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorRequiredIf, ValidatorRequiredUnless, ValidatorRequiredWith, ValidatorRequiredWithAll, ValidatorRequiredWithout, ValidatorRequiredWithoutAll
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorRequired : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new required validator.
     *
     * \param field     Name of the input field that is required.
     * \param messages  Custom error message if validation fails.
     */
    ValidatorRequired(const QString &field, const ValidatorMessages &messages = ValidatorMessages());

    /*!
     * \brief Deconstructs the required validator.
     */
    ~ValidatorRequired();

protected:
    /*!
     * \brief Performs the validation and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input parameter value as QString.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /*!
     * \brief Returns a generic error message.
     */
    QString genericValidationError(Context *c, const QVariant &errorData = QVariant()) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorRequired)
    Q_DISABLE_COPY(ValidatorRequired)
};

}

#endif //CUTELYSTVALIDATORREQUIRED_H
