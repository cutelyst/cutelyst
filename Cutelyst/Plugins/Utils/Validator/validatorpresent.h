/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORPRESENT_H
#define CUTELYSTVALIDATORPRESENT_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class ValidatorPresentPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorPresent validatorpresent.h <Cutelyst/Plugins/Utils/validatorpresent.h>
 * \brief The field under validation must be present in input data but can be empty.
 *
 * This validator checks if the \a field is present in the input data, but not if it contains any content.
 * If you want to check the fields presence and require it to have content, use one of the \link ValidatorRequired required validators \endlink.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorFilled, ValidatorRequired
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorPresent : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new present validator.
     * \param field         Name of the input field to validate.
     * \param messages      Custom error message if validation fails.
     */
    ValidatorPresent(const QString &field, const ValidatorMessages &messages = ValidatorMessages());

    /*!
     * \brief Deconstructs the present validator.
     */
    ~ValidatorPresent() override;

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

private:
    Q_DECLARE_PRIVATE(ValidatorPresent)
    Q_DISABLE_COPY(ValidatorPresent)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORPRESENT_H
