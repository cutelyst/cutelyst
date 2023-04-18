/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORREQUIREDUNLESS_H
#define CUTELYSTVALIDATORREQUIREDUNLESS_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class ValidatorRequiredUnlessPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorRequiredUnless validatorrequiredunless.h <Cutelyst/Plugins/Utils/validatorrequiredunless.h>
 * \brief The field under validation must be present and not empty unless the other field is equal to any value in the list.
 *
 * If the other field specified as \a otherField does \b not contain \b any of the values specified in the \a otherValues list, the
 * \a field under validation must be present and not empty. This validator is the opposite of ValidatorRequiredIf.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation. So, fields that only contain
 * whitespaces will be treated as empty.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorRequired, ValidatorRequiredIf, ValidatorRequiredWith, ValidatorRequiredWithAll, ValidatorRequiredWithout, ValidatorRequiredWithoutAll
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorRequiredUnless : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new required unless validator.
     * \param field         Name of the input field to validate.
     * \param otherField    Name of the other input field to validate.
     * \param otherValues   List of values that are not allowed to be in the other field to require the main field.
     * \param messages      Custom error messages if validation fails.
     */
    ValidatorRequiredUnless(const QString &field, const QString &otherField, const QStringList &otherValues, const ValidatorMessages &messages = ValidatorMessages());

    /*!
     * \brief Deconstructs the required unless validator.
     */
    ~ValidatorRequiredUnless() override;

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
    Q_DECLARE_PRIVATE(ValidatorRequiredUnless)
    Q_DISABLE_COPY(ValidatorRequiredUnless)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORREQUIREDUNLESS_H
