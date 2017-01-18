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
#ifndef CUTELYSTVALIDATORREQUIREDWITHOUT_H
#define CUTELYSTVALIDATORREQUIREDWITHOUT_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"
#include <QStringList>

namespace Cutelyst {
    
class ValidatorRequiredWithoutPrivate;

/*!
 * \brief The field under validation must be present and not empty only when any of the other specified fields are not present.
 *
 * If \b any of the fields in the \a otherFields list is \b not part of the input parameters, the \a field under validation must be present and not empty.
 * For the other fields it will only be checked if they are not present in the input parameters, not their content.
 *
 * If ValidatorRule::trimBefore() is set to \c true (the default), whitespaces will be removed from
 * the beginning and the end of the input value before validation. So, fields that only contain whitespaces
 * will be treated as empty and are invalid.
 *
 * \link Validator See Validator for general usage of validators. \endlink
 *
 * \sa ValidatorRequired, ValidatorRequiredIf, ValidatorRequiredUnless, ValidatorRequiredWith, ValidatorRequiredWithAll, ValidatorRequiredWithoutAll
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorRequiredWithout : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new required with validator.
     * \param field         Name of the input field to validate.
     * \param otherFields   List of other fields from which one has to be missing in the input to require the field.
     * \param label         Human readable input field label, used for generic error messages.
     * \param customError   Custom error message if validation fails.
     */
    ValidatorRequiredWithout(const QString &field, const QStringList &otherFields, const QString &label = QString(), const QString &customError = QString());
    
    /*!
     * \brief Deconstructs the required with validator.
     */
    ~ValidatorRequiredWithout();
    
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
    
    ValidatorRequiredWithout(ValidatorRequiredWithoutPrivate &dd);
    
private:
    Q_DECLARE_PRIVATE(ValidatorRequiredWithout)
    Q_DISABLE_COPY(ValidatorRequiredWithout)
};
    
}

#endif //CUTELYSTVALIDATORREQUIREDWITHOUT_H

