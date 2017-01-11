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
#ifndef CUTELYSTVALIDATORREQUIREDWITHALL_H
#define CUTELYSTVALIDATORREQUIREDWITHALL_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"
#include <QStringList>

namespace Cutelyst {
    
class ValidatorRequiredWithAllPrivate;

/*!
 * \brief The field under validation must be present and not empty only if all of the other specified fields are present.
 *
 * If \b all of the fields defined in the \a otherFields list are present in the input data, the \a field under validation must
 * be present and not empty. For the other fields only their presence will be checked, not their content.
 *
 * If ValidatorRule::trimBefore() is set to \c true (the default), whitespaces will be removed from
 * the beginning and the end of the input value before validation. So, fields that only contain whitespaces
 * will be treated as empty.
 *
 * \link Validator See Validator for general usage of validators. \endlink
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
     * \param label         Human readable input field label, used for generic error messages.
     * \param customError   Custom error message if validation fails.
     */
    ValidatorRequiredWithAll(const QString &field, const QStringList &otherFields, const QString &label = QString(), const QString &customError = QString());
    
    /*!
     * \brief Deconstructs the required with all validator.
     */
    ~ValidatorRequiredWithAll();
    
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
    
    ValidatorRequiredWithAll(ValidatorRequiredWithAllPrivate &dd);
    
private:
    Q_DECLARE_PRIVATE(ValidatorRequiredWithAll)
    Q_DISABLE_COPY(ValidatorRequiredWithAll)
};
    
}

#endif //CUTELYSTVALIDATORREQUIREDWITHALL_H

