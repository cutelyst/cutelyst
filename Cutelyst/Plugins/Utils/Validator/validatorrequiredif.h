/*
 * SPDX-FileCopyrightText: (C) 2017-2025 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORREQUIREDIF_H
#define CUTELYSTVALIDATORREQUIREDIF_H

#include "validatorrule.h"

#include <QStringList>

namespace Cutelyst {

class ValidatorRequiredIfPrivate;

/**
 * \ingroup plugins-utils-validator-rules
 * \headerfile "" <Cutelyst/Plugins/Utils/validatorrequiredif.h>
 * \brief The field under validation must be present and not empty if the other field is equal to
 * any value in a list.
 *
 * If the other field specified as \a otherField contains \b any of the values defined in the \a
 * otherValues list, the field under validation must be present and not empty. This validator is the
 * opposite of ValidatorRequiredUnless.
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
 * \sa ValidatorRequired, ValidatorRequiredUnless, ValidatorRequiredWith, ValidatorRequiredWithAll,
 * ValidatorRequiredWithout, ValidatorRequiredWithoutAll
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorRequiredIf : public ValidatorRule
{
public:
    /**
     * Constructs a new %ValidatorRequiredIf object with the given parameters.
     *
     * \param field         Name of the input field to validate.
     * \param otherField    Name of the other input field to validate.
     * \param otherValues   Values in the other field from which one must match the other field’s
     *                      content to require the main field.
     * \param messages      Custom error messages if validation fails.
     */
    ValidatorRequiredIf(const QString &field,
                        const QString &otherField,
                        const QStringList &otherValues,
                        const ValidatorMessages &messages = ValidatorMessages());

    /**
     * Destroys the %ValidatorRequiredIf object.
     */
    ~ValidatorRequiredIf() override;

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
    Q_DECLARE_PRIVATE(ValidatorRequiredIf) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    Q_DISABLE_COPY(ValidatorRequiredIf)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORREQUIREDIF_H
