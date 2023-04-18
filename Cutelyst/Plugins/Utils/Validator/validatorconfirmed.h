/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORCONFIRMED_H
#define CUTELYSTVALIDATORCONFIRMED_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class ValidatorConfirmedPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorConfirmed validatorconfirmed.h <Cutelyst/Plugins/Utils/validatorconfirmed.h>
 * \brief Checks for a confirmation input field.
 *
 * The \a field under validation must have a matching field of \c foo_confirmation. For example, if the field under
 * validation is \c password, a matching \c password_confirmation field must be present in the input with the same value.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation.
 * If the \a field's value is empty or if the \a field is missing in the input data, the validation will succeed without
 * performing the validation itself. Use one of the \link ValidatorRequired required validators \endlink to require the
 * field to be present and not empty.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorSame
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorConfirmed : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new confirmed validator.
     * \param field         Name of the input field to validate.
     * \param messages      Custom error message if validation fails.
     */
    ValidatorConfirmed(const QString &field, const ValidatorMessages &messages = ValidatorMessages());

    /*!
     * \brief Deconstructs the confirmed validator.
     */
    ~ValidatorConfirmed() override;

protected:
    /*!
     * \brief Performs the validation and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input paramater values as QString.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /*!
     * \brief Returns a generic error message if validation failed.
     */
    QString genericValidationError(Context *c, const QVariant &errorData = QVariant()) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorConfirmed)
    Q_DISABLE_COPY(ValidatorConfirmed)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORCONFIRMED_H
