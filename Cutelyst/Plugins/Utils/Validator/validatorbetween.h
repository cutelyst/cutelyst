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
#ifndef CUTELYSTVALIDATORBETWEEN_H
#define CUTELYSTVALIDATORBETWEEN_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"

namespace Cutelyst {
    
class ValidatorBetweenPrivate;
/*!
 * \brief Checks if a value or text length is between a minimum and maximum value.
 *
 * This validator has the mandatory extra parameters \a type, \a min and \a max. \a type specifies the type to validate against,
 * it can be QMetaType::Int, QMetaType::UInt, QMetaType::Float or QMetaType::QString, that are used as base types. You can compare against
 * any integer, floating point or string type. If you validate a string, it's length will be checked if it is between min and max values.
 *
 * The min and max values are converted internally into comparable values: qlonglong for signed integers, qulonglong for unsigned integers,
 * double for floating points and integer for string length.
 *
 * If ValidatorRule::trimBefore() is set to \c true (the default), whitespaces will be removed from
 * the beginning and the end of the input value before validation. If the \a field's value is empty or if
 * the \a field is missing in the input data, the validation will succeed without performing the validation itself.
 * Use one of the \link ValidatorRequired required validators \endlink to require the field to be present and not empty.
 *
 * \link Validator See Validator for general usage of validators. \endlink
 *
 * \par Example
 * \code{.cpp}
 * Validator v(params);
 * v.addValidator(new ValidatorBetween(QStringLiteral("username"), QMetaType::QString, 3, 255));
 * \endcode
 *
 * \sa ValidatorMin, ValidatorMax, ValidatorSize
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorBetween : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new between validator.
     * \param field         Name of the input field to validate.
     * \param type          The type to compare. Can be QMetaType::Int, QMetaType::UInt, QMetaType::Float or QMetaType::QString.
     * \param min           Minimum value. Will be converted into comparable value.
     * \param max           Maximum value. Will be converted into comparable value.
     * \param label         Human readable input field label, used for error messages.
     * \param customError   Custom errror message if validation fails.
     */
    ValidatorBetween(const QString &field, QMetaType::Type type, double min, double max, const QString &label = QString(), const QString &customError = QString());
    
    /*!
     * \brief Deconstructs the between validator.
     */
    ~ValidatorBetween();
    
    /*!
     * \brief Performs the validation and returns an empty QString on success, otherwise an error message.
     */
    QString validate() const override;

    /*!
     * \brief Sets the type to compare.
     *
     * Allowed values: QMetaType::Int, QMetaType::UInt, QMetaType::Float or QMetaType::QString.
     */
    void setType(QMetaType::Type type);

    /*!
     * \brief Sets the minimum value.
     */
    void setMin(double min);

    /*!
     * \brief Sets the maximum value.
     */
    void setMax(double max);
    
protected:
    /*!
     * \brief Returns a generic error message.
     */
    QString genericValidationError() const override;
    
    /*!
     * Constructs a new ValidatorBetween object with the given private class.
     */
    ValidatorBetween(ValidatorBetweenPrivate &dd);
    
private:
    Q_DECLARE_PRIVATE(ValidatorBetween)
    Q_DISABLE_COPY(ValidatorBetween)
};
    
}

#endif //CUTELYSTVALIDATORBETWEEN_H

