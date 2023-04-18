/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORREQUIRED_H
#define CUTELYSTVALIDATORREQUIRED_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class ValidatorRequiredPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorRequired validatorrequired.h <Cutelyst/Plugins/Utils/validatorrequired.h>
 * \brief Checks if a field is available and not empty.
 *
 * The \a field under validation must be present in the input data and not empty.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation.
 * So, fields that only contain whitespaces will be treated as empty and are invalid.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorRequiredIf, ValidatorRequiredUnless, ValidatorRequiredWith, ValidatorRequiredWithAll, ValidatorRequiredWithout, ValidatorRequiredWithoutAll
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorRequired : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new required validator.
     *
     * \param field     Name of the input field that is required.
     * \param messages  Custom error message if validation fails.
     */
    ValidatorRequired(const QString &field, const ValidatorMessages &messages = ValidatorMessages());

    /*!
     * \brief Deconstructs the required validator.
     */
    ~ValidatorRequired() override;

protected:
    /*!
     * \brief Performs the validation and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input parameter value as QString.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /*!
     * \brief Returns a generic error message.
     */
    QString genericValidationError(Context *c, const QVariant &errorData = QVariant()) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorRequired)
    Q_DISABLE_COPY(ValidatorRequired)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORREQUIRED_H
