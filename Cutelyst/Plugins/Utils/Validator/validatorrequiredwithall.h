/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORREQUIREDWITHALL_H
#define CUTELYSTVALIDATORREQUIREDWITHALL_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

#include <QStringList>

namespace Cutelyst {

class ValidatorRequiredWithAllPrivate;

/**
 * \ingroup plugins-utils-validator-rules
 * \headerfile "" <Cutelyst/Plugins/Utils/validatorrequiredwithall.h>
 * \brief The field under validation must be present and not empty only if all of the other
 * specified fields are present.
 *
 * If \b all of the fields defined in the \a otherFields list are present in the input data, the
 * \a field under validation must be present and not empty. For the other fields only their
 * presence will be checked, not their content.
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
 * ValidatorRequiredWithout, ValidatorRequiredWithoutAll
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorRequiredWithAll : public ValidatorRule
{
public:
    /**
     * Constructs a new %ValidatorRequiredWithAll object with the given parameters.
     *
     * \param field         Name of the input field to validate.
     * \param otherFields   List of fields that mus all be present in the input data to require
     *                      the \a field.
     * \param messages      Custom error messages if validation fails.
     */
    ValidatorRequiredWithAll(const QString &field,
                             const QStringList &otherFields,
                             const ValidatorMessages &messages = ValidatorMessages());

    /**
     * Destroys the %ValidatorRequiredWithAll object.
     */
    ~ValidatorRequiredWithAll() override;

protected:
    /**
     * Performs the validation on the input \a params and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input paramter
     * value as QString.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /**
     * Returns a generic error message if validation failed.
     */
    QString genericValidationError(Context *c,
                                   const QVariant &errorData = QVariant()) const override;

private:
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    Q_DECLARE_PRIVATE(ValidatorRequiredWithAll)
    Q_DISABLE_COPY(ValidatorRequiredWithAll)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORREQUIREDWITHALL_H
