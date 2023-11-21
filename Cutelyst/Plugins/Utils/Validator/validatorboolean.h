/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORBOOLEAN_H
#define CUTELYSTVALIDATORBOOLEAN_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class ValidatorBooleanPrivate;

/**
 * \ingroup plugins-utils-validator-rules
 * \headerfile "" <Cutelyst/Plugins/Utils/validatorboolean.h>
 * \brief Checks if a value can be casted into a boolean.
 *
 * The \a field under validation must contain one of the following acceptable input values: \c 1, \c
 * 0, \c true, \c false, \c on or \c off.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link
 * Validator::NoTrimming NoTrimming\endlink, whitespaces will be removed from the beginning and the
 * end of the input value before validation. If the \a field's value is empty or if the \a field is
 * missing in the input data, the validation will succeed without performing the validation itself.
 * Use one of the \link ValidatorRequired required validators \endlink to require the field to be
 * present and not empty.
 *
 * \par Return type
 * On success, ValidatorReturnType::value will contain a bool value.
 *
 * \sa Validator for general usage of validators.
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorBoolean : public ValidatorRule
{
public:
    /**
     * Constructs a new %ValidatorBoolean with the given parameters.
     *
     * \param field     Name of the input field to validate.
     * \param messages  Custom error message if validation fails.
     * \param defValKey \link Context::stash() Stash \endlink key containing a default value if
     *                  input field is empty. This value will \b NOT be validated.
     */
    ValidatorBoolean(const QString &field,
                     const ValidatorMessages &messages = ValidatorMessages(),
                     const QString &defValKey          = QString());

    /**
     * Destroys the %ValidatorBoolean object.
     */
    ~ValidatorBoolean() override;

protected:
    /**
     * Performs the validation on the input \a params and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will either contain \c true if the
     * input value contains \c 1, \c true or \on, or \c false if value contains \c 0, \c false or \c
     * off.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /**
     * Returns a generic error message if validation failed.
     */
    QString genericValidationError(Context *c,
                                   const QVariant &errorData = QVariant()) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorBoolean) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    Q_DISABLE_COPY(ValidatorBoolean)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORBOOLEAN_H
