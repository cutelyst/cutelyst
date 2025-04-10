/*
 * SPDX-FileCopyrightText: (C) 2017-2025 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORNUMERIC_H
#define CUTELYSTVALIDATORNUMERIC_H

#include "validatorrule.h"

namespace Cutelyst {

class ValidatorNumericPrivate;

/**
 * \ingroup plugins-utils-validator-rules
 * \headerfile "" <Cutelyst/Plugins/Utils/validatornumeric.h>
 * \brief Checks if the field under validation could be casted into a numeric value.
 *
 * Checks for signed and unsigned integers as well as floats (also with exponential e) together
 * with minus signs in the input \a field. Valid values are in format 3, -3.54, 8.89234e12 etc.
 * Internally this will simply try to convert the parameter value from a QString into a double.
 *
 * \note Conversion of numeric input values is performed in the \c 'C' locale.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link
 * Validator::NoTrimming NoTrimming\endlink, whitespaces will be removed from the beginning and the
 * end of the input value before validation. If the \a field's value is empty or if the \a field is
 * missing in the input data, the validation will succeed without performing the validation itself.
 * Use one of the \link ValidatorRequired required validators \endlink to require the field to be
 * present and not empty.
 *
 * \par Return type
 * On success, ValidatorReturnType::value will contain a double.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorInteger
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorNumeric : public ValidatorRule
{
public:
    /**
     * Constructs a new %ValidatorNumeric with the given parameters.
     *
     * \param field         Name of the input field to validate.
     * \param messages      Custom error message if validation fails.
     * \param defValKey     \link Context::stash() Stash \endlink key containing a default value
     *                      if input field is empty. This value will \b NOT be validated.
     */
    explicit ValidatorNumeric(const QString &field,
                              const ValidatorMessages &messages = {},
                              const QString &defValKey          = {});

    /**
     * Destroys the %ValidatorNumeric object.
     */
    ~ValidatorNumeric() override;

protected:
    /**
     * Performs the validation on the input \a params and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input paramter
     * value converted into a double.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /**
     * Performs the validation on the input \a params and calls the \a cb with the
     * ValidatorReturnType as argument.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input paramter
     * value converted into a double.
     *
     * \since Cutelyst 5.0.0
     */
    void validateCb(Context *c, const ParamsMultiMap &params, ValidatorRtFn cb) const override;

    /**
     * Returns a generic error message if validation failed.
     */
    QString genericValidationError(Context *c,
                                   const QVariant &errorData = QVariant()) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorNumeric) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    Q_DISABLE_COPY(ValidatorNumeric)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORNUMERIC_H
