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
#ifndef CUTELYSTVALIDATORDIGITSBETWEEN_H
#define CUTELYSTVALIDATORDIGITSBETWEEN_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"

namespace Cutelyst {
    
class ValidatorDigitsBetweenPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorDigitsBetween validatordigitsbetween.h <Cutelyst/Plugins/Utils/validatordigitsbetween.h>
 * \brief Checks for digits only with a length between min and max.
 *
 * The field under validation must only contain digits with a length between \a min and \a max. The digits
 * are not interpreteded as a numeric value but as a string, so the \a min and \a max values are not a range
 * for a numeric value but for the string length.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation.
 * If the \a field's value is empty or if the \a field is missing in the input data, the validation will succeed without
 * performing the validation itself. Use one of the \link ValidatorRequired required validators \endlink to require the
 * field to be present and not empty.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorDigits
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorDigitsBetween : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new digits between validator.
     * \param field         Name of the input field to validate.
     * \param min           Minimum length of the digits. Should either be an integer value to directly specify the length or the name of a \link Context::stash() Stash\endlink key containing the length constraint.
     * \param max           Maximum length of the digits. Should either be an integer value to directly specify the length or the name of a \link Context::stash() Stash\endlink key containing the length constraint.
     * \param messages      Custom error messages if validation fails.
     * \param defValKey     \link Context::stash() Stash \endlink key containing a default value if input field is empty. This value will \b NOT be validated.
     */
    ValidatorDigitsBetween(const QString &field, const QVariant &min, const QVariant &max, const ValidatorMessages &messages = ValidatorMessages(), const QString &defValKey = QString());
    
    /*!
     * \brief Deconstructs the digits between validator.
     */
    ~ValidatorDigitsBetween();

    /*!
     * \ingroup plugins-utils-validator-rules
     * \brief Returns \c true if \a value only contains digits and has a length between \a min and \a max.
     * \param value The value to validate as it is.
     * \param min   Minimum length of the digits.
     * \param max   Maximum length of the digits.
     * \return \c true if \a value string only contains digits and has a length between \a min and \a max, otherwise it returns \a false.
     * Nothe that this might return \c true for an empty value if 0 is between \a min and \a max.
     */
    static bool validate(const QString &value, int min, int max);
    
protected:    
    /*!
     * \brief Performs the validation and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input parameter value as QString.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /*!
     * \brief Returns a generic error if validation failed.
     *
     * \a errorData will contain a QVariantList with the \a min value as first and the \a max value as second entry.
     */
    QString genericValidationError(Context *c, const QVariant &errorData = QVariant()) const override;
    
private:
    Q_DECLARE_PRIVATE(ValidatorDigitsBetween)
    Q_DISABLE_COPY(ValidatorDigitsBetween)
};
    
}

#endif //CUTELYSTVALIDATORDIGITSBETWEEN_H

