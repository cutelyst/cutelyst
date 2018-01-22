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
#ifndef CUTELYSTVALIDATORREQUIREDWITHOUTALL_H
#define CUTELYSTVALIDATORREQUIREDWITHOUTALL_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"
#include <QStringList>

namespace Cutelyst {
    
class ValidatorRequiredWithoutAllPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \brief The field under validation must be present and not empty only when all of the other specified fields are not present.
 *
 * If \b all of the fields specified in the \a otherFields list are \b not present in the input data, the \a field under validation
 * must be present and not empty. For the other fields it will only be checked if they are not part of the input data, not their content.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation. So, fields that only contain
 * whitespaces will be treated as empty.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorRequired, ValidatorRequiredIf, ValidatorRequiredUnless, ValidatorRequiredWith, ValidatorRequiredWithAll, ValidatorRequiredWithout
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorRequiredWithoutAll : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new required without all validator.
     * \param field         Name of the input field to validate.
     * \param otherFields   List of field names that are not allowed to be present to require the field.
     * \param messages      Custom error messages if validation fails.
     */
    ValidatorRequiredWithoutAll(const QString &field, const QStringList &otherFields, const ValidatorMessages &messages = ValidatorMessages());
    
    /*!
     * \brief Deconstructs the required without all validator.
     */
    ~ValidatorRequiredWithoutAll();
    
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
    Q_DECLARE_PRIVATE(ValidatorRequiredWithoutAll)
    Q_DISABLE_COPY(ValidatorRequiredWithoutAll)
};
    
}

#endif //CUTELYSTVALIDATORREQUIREDWITHOUTALL_H

