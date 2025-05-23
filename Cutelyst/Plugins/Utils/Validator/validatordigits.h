/*
 * SPDX-FileCopyrightText: (C) 2017-2025 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORDIGITS_H
#define CUTELYSTVALIDATORDIGITS_H

#include "validatorrule.h"

namespace Cutelyst {

class ValidatorDigitsPrivate;

/**
 * \ingroup plugins-utils-validator-rules
 * \headerfile "" <Cutelyst/Plugins/Utils/validatordigits.h>
 * \brief Checks for digits only with optional length check.
 *
 * The \a field under validation must only contain digits with an optional exact \a length. If
 * length is set to a value lower or equal to \c 0, the length check will not be performed. The
 * input digits will not be interpreted as numeric values but as a string. So the length is not
 * meant to test for an exact numeric value but for the string length. The \a length can be either
 * specified as fixed value or as a QString naming a stash key that contains the length value.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link
 * Validator::NoTrimming NoTrimming\endlink, whitespaces will be removed from the beginning and the
 * end of the input value before validation. If the \a field's value is empty or if the \a field is
 * missing in the input data, the validation will succeed without performing the validation itself.
 * Use one of the \link ValidatorRequired required validators \endlink to require the field to be
 * present and not empty.
 *
 * \par Return type
 * On success, ValidatorReturnType::value will contain a QString.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorDigitsBetween
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorDigits : public ValidatorRule
{
public:
    /**
     * Constructs a new %ValidatorDigits with the given parameters.
     *
     * \param field         Name of the input field to validate.
     * \param length        Exact length of the digits, defaults to \c -1. A value lower \c 1
     *                      disables the length check. Should be either an int to directly specify
     *                      the length or the name of a \link Context::stash() Stash \endlink key
     *                      containing the length constraint.
     * \param messages      Custom error messages if validation fails.
     * \param defValKey     \link Context::stash() Stash \endlink key containing a default value
     *                      if input field is empty. This value will \b NOT be validated.
     */
    explicit ValidatorDigits(const QString &field,
                             const QVariant &length            = -1,
                             const ValidatorMessages &messages = {},
                             const QString &defValKey          = {});

    /**
     * Destroys the %ValidatorDigits object.
     */
    ~ValidatorDigits() override;

    /**
     * \ingroup plugins-utils-validator-rules
     * \brief Returns \c true if \a value only contains digits.
     *
     * Note that this function will return \c true for an empty \a value if the \a length check is
     * disabled.
     *
     * \param value     The value to validate as it is.
     * \param length    Exact length of the digits, defaults to \c -1. A value lower \c 1 disables
     *                  the length check.
     * \return \c true if the \a value only contains digits
     */
    static bool validate(const QString &value, qsizetype length = -1);

protected:
    /**
     * Performs the validation on the input \a params and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input parameter value as
     * QString.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /**
     * Performs the validation on the input \a params and calls the \a cb with the
     * ValidatorReturnType as argument.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input parameter value as
     * QString.
     *
     * \since Cutelyst 5.0.0
     */
    void validateCb(Context *c, const ParamsMultiMap &params, ValidatorRtFn cb) const override;

    /**
     * Returns a generic error if validation failed.
     */
    QString genericValidationError(Context *c,
                                   const QVariant &errorData = QVariant()) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorDigits) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    Q_DISABLE_COPY(ValidatorDigits)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORDIGITS_H
