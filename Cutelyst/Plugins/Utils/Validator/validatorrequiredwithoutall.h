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
#ifndef CUTELYSTVALIDATORREQUIREDWITHOUTALL_H
#define CUTELYSTVALIDATORREQUIREDWITHOUTALL_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"
#include <QStringList>

namespace Cutelyst {
    
class ValidatorRequiredWithoutAllPrivate;

/*!
 * \brief The field under validation must be present and not empty only when all of the other specified fields are not present.
 *
 * If \b all of the fields specified in the \a otherFields list are \b not present in the input data, the \a field under validation
 * must be present and not empty. For the other fields it will only be checked if they are not part of the input data, not their content.
 *
 * If ValidatorRule::trimBefore() is set to \c true (the default), whitespaces will be removed from
 * the beginning and the end of the input value before validation. So, fields that only contain whitespaces
 * will be treated as empty.
 *
 * \link Validator See Validator for general usage of validators. \endlink
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
     * \param label         Human readable input field label, used for generic error messages.
     * \param customError   Custom error message if validation fails.
     */
    ValidatorRequiredWithoutAll(const QString &field, const QStringList &otherFields, const QString &label = QString(), const QString &customError = QString());
    
    /*!
     * \brief Deconstructs the required without all validator.
     */
    ~ValidatorRequiredWithoutAll();
    
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
     * Constructs a new ValidatorRequiredWithoutAll object with the given private class.
     */
    ValidatorRequiredWithoutAll(ValidatorRequiredWithoutAllPrivate &dd);
    
private:
    Q_DECLARE_PRIVATE(ValidatorRequiredWithoutAll)
    Q_DISABLE_COPY(ValidatorRequiredWithoutAll)
};
    
}

#endif //CUTELYSTVALIDATORREQUIREDWITHOUTALL_H

