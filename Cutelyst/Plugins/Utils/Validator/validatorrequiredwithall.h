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
#ifndef CUTELYSTVALIDATORREQUIREDWITHALL_H
#define CUTELYSTVALIDATORREQUIREDWITHALL_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"
#include <QStringList>

namespace Cutelyst {
    
class ValidatorRequiredWithAllPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorRequiredWithAll validatorrequiredwithall.h <Cutelyst/Plugins/Utils/validatorrequiredwithall.h>
 * \brief The field under validation must be present and not empty only if all of the other specified fields are present.
 *
 * If \b all of the fields defined in the \a otherFields list are present in the input data, the \a field under validation must
 * be present and not empty. For the other fields only their presence will be checked, not their content.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation. So, fields that only contain
 * whitespaces will be treated as empty.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorRequired, ValidatorRequiredIf, ValidatorRequiredUnless, ValidatorRequiredWith, ValidatorRequiredWithout, ValidatorRequiredWithoutAll
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorRequiredWithAll : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new required with all validator.
     * \param field         Name of the input field to validate.
     * \param otherFields   List of fields that mus all be present in the input data to require the field.
     * \param messages      Custom error messages if validation fails.
     */
    ValidatorRequiredWithAll(const QString &field, const QStringList &otherFields, const ValidatorMessages &messages = ValidatorMessages());
    
    /*!
     * \brief Deconstructs the required with all validator.
     */
    ~ValidatorRequiredWithAll();
    
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
    Q_DECLARE_PRIVATE(ValidatorRequiredWithAll)
    Q_DISABLE_COPY(ValidatorRequiredWithAll)
};
    
}

#endif //CUTELYSTVALIDATORREQUIREDWITHALL_H

