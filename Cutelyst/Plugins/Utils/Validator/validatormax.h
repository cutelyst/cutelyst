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
#ifndef CUTELYSTVALIDATORMAX_H
#define CUTELYSTVALIDATORMAX_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"
#include <QObject>

namespace Cutelyst {
    
class ValidatorMaxPrivate;

/*!
 * \brief Checks if a value is not bigger or longer than a maximum value.
 *
 * This works for floating point, integer and QString types, where for numeric types it will check the value itself while for QString it will check the string length.
 * Use \a max to define the maximum value and \a type to set the type to check against. \a max will internally converted into a comparative value (qlonglong for QMetaType::Int,
 * qulonglong for QMetaType::UInt and int for QMetaType::QString. Allowed types for the \a type specifier are QMetaType::Int, QMetaType::UInt, QMetaType::Float and QMetaType::QString.
 * Any other type will lead to an validation data error. These types can be seen as base types:
 *
 * \li QMetaType::UInt for all unsigned integer types, converted into qulonglong
 * \li QMetaType::Int for all signed integer types, converted into qlonglong
 * \li QMetaType::Float for all floating point types, converted into double
 * \li QMetaType::QString for what the name says, converted into int
 *
 * If ValidatorRule::trimBefore() is set to \c true (the default), whitespaces will be removed from
 * the beginning and the end of the input value before validation. If the \a field's value is empty or if
 * the \a field is missing in the input data, the validation will succeed without performing the validation itself.
 * Use one of the \link ValidatorRequired required validators \endlink to require the field to be present and not empty.
 *
 * \link Validator See Validator for general usage of validators. \endlink
 *
 * \sa ValidatorMin, ValidatorSize, ValidatorBetween
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorMax : public ValidatorRule
{
    Q_OBJECT
public:
    /*!
     * \brief Constructs a new max validator.
     * \param field         Name of the input field to validate.
     * \param type          The type to compare.
     * \param max           Maximum value.
     * \param label         Human readable input field label, used for generic error messages.
     * \param customError   Custom error message if validation fails.
     * \param parent        Parent object.
     */
    ValidatorMax(const QString &field, QMetaType::Type type, double max, const QString &label = QString(), const QString &customError = QString(), QObject *parent = nullptr);
    
    /*!
     * \brief Deconstructs the max validator.
     */
    ~ValidatorMax();
    
    /*!
     * \brief Performs the validation.
     *
     * Returns \c true on success.
     */
    bool validate() override;

    /*!
     * \brief Sets the type to validate.
     */
    void setType(QMetaType::Type type);

    /*!
     * \brief Sets the maximum value.
     */
    void setMax(double max);
    
protected:
    /*!
     * \brief Returns a generic error message.
     */
    QString genericErrorMessage() const override;
    
    ValidatorMax(ValidatorMaxPrivate &dd, QObject *parent);
    
private:
    Q_DECLARE_PRIVATE(ValidatorMax)
    Q_DISABLE_COPY(ValidatorMax)
};
    
}

#endif //CUTELYSTVALIDATORMAX_H

