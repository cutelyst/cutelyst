/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORINTEGER_H
#define CUTELYSTVALIDATORINTEGER_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class ValidatorIntegerPrivate;

/**
 * \ingroup plugins-utils-validator-rules
 * \headerfile "" <Cutelyst/Plugins/Utils/validatorinteger.h>
 * \brief Checks if the value is an integer.
 *
 * Tries to convert the input parameter value into the integer \a type specified in the constructor.
 * If the input value does not fit into the \a type, the validation fails.
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
 * On success, ValidatorReturnType::value will contain an integer \a type set in the constructor.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorNumeric
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorInteger : public ValidatorRule
{
public:
    /**
     * Constructs a new %ValidatorInteger object with the given parameters.
     *
     * \param field         Name of the input field to validate.
     * \param type          The type the integer value should fit in. Only integer types are
     *                      supported, everything else will generate a validation data error.
     * \param messages      Custom error message if validation fails.
     * \param defValKey     \link Context::stash() Stash \endlink key containing a default value
     *                      if input field is empty. This value will \b NOT be validated.
     */
    ValidatorInteger(const QString &field,
                     QMetaType::Type type              = QMetaType::ULongLong,
                     const ValidatorMessages &messages = ValidatorMessages(),
                     const QString &defValKey          = QString());

    /**
     * Destroys the %ValidatorInteger object.
     */
    ~ValidatorInteger() override;

protected:
    /**
     * Performs the validation on the input \a params and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input paramter
     * value as the type set in the constructor.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /**
     * \brief Returns a generic error message if validation failed.
     */
    QString genericValidationError(Context *c,
                                   const QVariant &errorData = QVariant()) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorInteger) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    Q_DISABLE_COPY(ValidatorInteger)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORINTEGER_H
