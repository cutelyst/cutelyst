/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORDIFFERENT_H
#define CUTELYSTVALIDATORDIFFERENT_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class ValidatorDifferentPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorDifferent validatordifferent.h <Cutelyst/Plugins/Utils/validatordifferent.h>
 * \brief Checks if two values are different.
 *
 * This will check if the value in the one input \a field is different from the value in the \a other input field.
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
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorDifferent : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new different validator.
     * \param field         Name of the input field to validate.
     * \param other         Name of the other field to compare against.
     * \param otherLabel    Translatable label of the other input field, used for generic error messages.
     * \param messages      Custom error messages if validation fails.
     */
    ValidatorDifferent(const QString &field, const QString &other, const char *otherLabel = nullptr, const ValidatorMessages &messages = ValidatorMessages());

    /*!
     * \brief Deconstructs the different validator.
     */
    ~ValidatorDifferent() override;

protected:
    /*!
     * \brief Performs the validation and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input paramter value as QString.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /*!
     * \brief Returns a generic error if validation failed.
     */
    QString genericValidationError(Context *c, const QVariant &errorData = QVariant()) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorDifferent)
    Q_DISABLE_COPY(ValidatorDifferent)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORDIFFERENT_H
