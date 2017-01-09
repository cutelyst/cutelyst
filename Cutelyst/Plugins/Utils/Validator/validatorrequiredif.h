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
#ifndef CUTELYSTVALIDATORREQUIREDIF_H
#define CUTELYSTVALIDATORREQUIREDIF_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"
#include <QObject>
#include <QStringList>

namespace Cutelyst {
    
class ValidatorRequiredIfPrivate;

/*!
 * \brief The field under validation must be present and not empty if the other field is equal to any value in the list.
 *
 * If the other field specified as \a otherField contains \b any of the values defined in the \a otherValues list, the
 * field under validation must be present and not empty.
 *
 * If ValidatorRule::trimBefore() is set to \c true (the default), whitespaces will be removed from
 * the beginning and the end of the input value before validation. So, fields that only contain whitespaces
 * will be treated as empty.
 *
 * \link Validator See Validator for general usage of validators. \endlink
 *
 * \sa ValidatorRequired, ValidatorRequiredUnless, ValidatorRequiredWith, ValidatorRequiredWithAll, ValidatorRequiredWithout, ValidatorRequiredWithoutAll
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorRequiredIf : public ValidatorRule
{
    Q_OBJECT
public:
    /*!
     * \brief Constructs a new required if validator.
     * \param field         Name of the input field to validate.
     * \param otherField    Name of the other input field to validate.
     * \param otherValues   Values in the other field from which one must match the field content to require the main field.
     * \param label         Human readable input field label, used for generic error messages.
     * \param customError   Custom error message if validation fails.
     * \param parent        Parent object.
     */
    ValidatorRequiredIf(const QString &field, const QString &otherField, const QStringList &otherValues, const QString &label = QString(), const QString &customError = QString(), QObject *parent = nullptr);
    
    /*!
     * \brief Deconstructs the required if validator.
     */
    ~ValidatorRequiredIf();
    
    /*!
     * \brief Performs the validation.
     *
     * Returns \c true on success.
     */
    bool validate() override;

    /*!
     * \brief Sets the name of the other field.
     */
    void setOtherField(const QString &otherField);

    /*!
     * \brief Sets the values that have to be in the other field.
     */
    void setOtherValues(const QStringList &otherValues);

    
protected:
    /*!
     * \brief Returns a generic error message.
     */
    QString genericErrorMessage() const override;
    
    ValidatorRequiredIf(ValidatorRequiredIfPrivate &dd, QObject *parent);
    
private:
    Q_DECLARE_PRIVATE(ValidatorRequiredIf)
    Q_DISABLE_COPY(ValidatorRequiredIf)
};
    
}

#endif //CUTELYSTVALIDATORREQUIREDIF_H

