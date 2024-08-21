/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORFILLED_H
#define CUTELYSTVALIDATORFILLED_H

#include "validatorrule.h"

namespace Cutelyst {

class ValidatorFilledPrivate;

/**
 * \ingroup plugins-utils-validator-rules
 * \headerfile "" <Cutelyst/Plugins/Utils/validatorfilled.h>
 * \brief The field under validation must not be empty when it is present.
 *
 * The difference to the \link ValidatorRequired required validator \endlink is, that it will only
 * be checked for non emptyness if it is available. If it is available, it is not allowed to be
 * empty.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link
 * Validator::NoTrimming NoTrimming\endlink, whitespaces will be removed from the beginning and the
 * end of the input value before validation.
 *
 * \par Return type
 * On success, ValidatorReturnType::value will contain a QString.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorPresent
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorFilled : public ValidatorRule
{
public:
    /**
     * Constructs a new %ValidatorFilled object with the given parameters.
     *
     * \param field     Name of the input field to validate.
     * \param messages  Custom error message if validation fails.
     */
    ValidatorFilled(const QString &field,
                    const ValidatorMessages &messages = ValidatorMessages(),
                    const QString &defValKey          = QString());

    /**
     * Destroys the %ValidatorFilled object.
     */
    ~ValidatorFilled() override;

protected:
    /**
     * Performs the validation on the input \a params and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input parameter value as
     * QString.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /**
     * Creates a generic error message.
     */
    QString genericValidationError(Context *c,
                                   const QVariant &errorData = QVariant()) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorFilled) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    Q_DISABLE_COPY(ValidatorFilled)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORFILLED_H
