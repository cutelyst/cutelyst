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
#ifndef CUTELYSTVALIDATORREQUIREDWITH_H
#define CUTELYSTVALIDATORREQUIREDWITH_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"
#include <QStringList>

namespace Cutelyst {
    
class ValidatorRequiredWithPrivate;


/*!
 * \brief The field under validation must be present and not empty only if any of the other specified fields are present.
 *
 * If \b any of the fields defined in the \a otherFields list is present in the input data, the \a field under validation must
 * be present and not empty. For the other fields only their presence in the input data will be checked, not their content.
 *
 * If ValidatorRule::trimBefore() is set to \c true (the default), whitespaces will be removed from
 * the beginning and the end of the input value before validation. So, fields that only contain whitespaces
 * will be treated as empty.
 *
 * \link Validator See Validator for general usage of validators. \endlink
 *
 * \sa ValidatorRequired, ValidatorRequiredIf, ValidatorRequiredUnless, ValidatorRequiredWithAll, ValidatorRequiredWithout, ValidatorRequiredWithoutAll
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorRequiredWith : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new required with validator.
     * \param field         Name of the input field to validate.
     * \param otherFields   List of other fields from which one must be present in the input data to require the field.
     * \param label         Human readable input field label, used for generic error messages.
     * \param customError   Custom error message if validation fails.
     */
    ValidatorRequiredWith(const QString &field, const QStringList &otherFields, const QString &label = QString(), const QString &customError = QString());
    
    /*!
     * \brief Deconstructs the required with validator.
     */
    ~ValidatorRequiredWith();
    
    /*!
     * \brief Performs the validation and returns an empty QString on success, otherwise an error message.
     */
    QString validate() const override;

    /*!
     * \brief Sets the list of other fields.
     */
    void setOtherFields(const QStringList &otherFields);
    
protected:
    /*!
     * \brief Returns a generic error message.
     */
    QString genericValidationError() const override;
    
    /*!
     * Constructs a new ValidatorRequiredWith object with the given private class.
     */
    ValidatorRequiredWith(ValidatorRequiredWithPrivate &dd);
    
private:
    Q_DECLARE_PRIVATE(ValidatorRequiredWith)
    Q_DISABLE_COPY(ValidatorRequiredWith)
};
    
}

#endif //CUTELYSTVALIDATORREQUIREDWITH_H

