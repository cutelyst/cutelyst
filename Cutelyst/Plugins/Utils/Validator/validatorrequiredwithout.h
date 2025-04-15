/*
 * SPDX-FileCopyrightText: (C) 2017-2025 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORREQUIREDWITHOUT_H
#define CUTELYSTVALIDATORREQUIREDWITHOUT_H

#include "validatorrule.h"

#include <QStringList>

namespace Cutelyst {

class ValidatorRequiredWithoutPrivate;

/**
 * \ingroup plugins-utils-validator-rules
 * \headerfile "" <Cutelyst/Plugins/Utils/validatorrequiredwithout.h>
 * \brief The field under validation must be present and not empty only if any of the other
 * specified fields is not present.
 *
 * If \b any of the fields in the \a otherFields list is \b not part of the input parameters, the \a
 * field under validation must be present and not empty. For the other fields it will only be
 * checked if they are not present in the input parameters, not their content.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link
 * Validator::NoTrimming NoTrimming\endlink, whitespaces will be removed from the beginning and the
 * end of the input value before validation. So, fields that only contain whitespaces will be
 * treated as empty.
 *
 * \par Return type
 * On success, ValidatorReturnType::value will contain a QString.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorRequired, ValidatorRequiredIf, ValidatorRequiredUnless, ValidatorRequiredWith,
 * ValidatorRequiredWithAll, ValidatorRequiredWithoutAll
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorRequiredWithout : public ValidatorRule
{
public:
    /**
     * Constructs a new %ValidatorRequiredWithout object with the given parameters.
     *
     * \param field         Name of the input field to validate.
     * \param otherFields   List of other fields from which one has to be missing in the input to
     *                      require the field.
     * \param messages      Custom error messages if validation fails.
     */
    ValidatorRequiredWithout(const QString &field,
                             const QStringList &otherFields,
                             const ValidatorMessages &messages = ValidatorMessages());

    /**
     * Destroys the %ValidatorRequiredWithout object.
     */
    ~ValidatorRequiredWithout() override;

protected:
    /**
     * Performs the validation on the input \a params and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input paramter
     * value as QString.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /**
     * Performs the validation on the input \a params and calls the \a cb with the
     * ValidatorReturnType as argument.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input paramter
     * value as QString.
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
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    Q_DECLARE_PRIVATE(ValidatorRequiredWithout)
    Q_DISABLE_COPY(ValidatorRequiredWithout)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORREQUIREDWITHOUT_H
