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
#ifndef CUTELYSTVALIDATORIN_H
#define CUTELYSTVALIDATORIN_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"

namespace Cutelyst {

class ValidatorInPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorIn validatorin.h <Cutelyst/Plugins/Utils/validatorin.h>
 * \brief Checks if the field value is one from a list of values.
 *
 * Validates if the \a field contains a value from the \a values list.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation.
 * If the \a field's value is empty or if the \a field is missing in the input data, the validation will succeed without
 * performing the validation itself. Use one of the \link ValidatorRequired required validators \endlink to require the
 * field to be present and not empty.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorNotIn
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorIn : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new in validator.
     * \param field         Name of the input field to validate.
     * \param values        List of values to compare against. Can be either a QStringList containing the allowed values or a QString specifing a stash key containing a QStringList with allowed values.
     * \param cs            Defines if the comparison should be performed case sensitive or insensitive.
     * \param messages      Custom error message if validation fails.
     * \param defValKey     \link Context::stash() Stash \endlink key containing a default value if input field is empty. This value will \b NOT be validated.
     */
    ValidatorIn(const QString &field, const QVariant &values, Qt::CaseSensitivity cs = Qt::CaseSensitive, const ValidatorMessages &messages = ValidatorMessages(), const QString &defValKey = QString());

    /*!
     * \brief Deconstructs the in validator.
     */
    ~ValidatorIn();

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

    /*!
     * \brief Returns a generic error messages if the list of comparison values is empty.
     */
    QString genericValidationDataError(Context *c, const QVariant &errorData) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorIn)
    Q_DISABLE_COPY(ValidatorIn)
};

}

#endif //CUTELYSTVALIDATORIN_H

