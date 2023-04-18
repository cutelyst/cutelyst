/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORREQUIREDWITH_H
#define CUTELYSTVALIDATORREQUIREDWITH_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

#include <QStringList>

namespace Cutelyst {

class ValidatorRequiredWithPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorRequiredWith validatorrequiredwith.h <Cutelyst/Plugins/Utils/validatorrequiredwith.h>
 * \brief The field under validation must be present and not empty only if any of the other specified fields is present.
 *
 * If \b any of the fields defined in the \a otherFields list is present in the input data, the \a field under validation must
 * be present and not empty. For the other fields only their presence in the input data will be checked, not their content.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation. So, fields that only contain
 * whitespaces will be treated as empty.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorRequired, ValidatorRequiredIf, ValidatorRequiredUnless, ValidatorRequiredWithAll, ValidatorRequiredWithout, ValidatorRequiredWithoutAll
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorRequiredWith : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new required with validator.
     * \param field         Name of the input field to validate.
     * \param otherFields   List of other fields from which one must be present in the input data to require the field.
     * \param messages      Custom error messages if validation fails.
     */
    ValidatorRequiredWith(const QString &field, const QStringList &otherFields, const ValidatorMessages &messages = ValidatorMessages());

    /*!
     * \brief Deconstructs the required with validator.
     */
    ~ValidatorRequiredWith() override;

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
    Q_DECLARE_PRIVATE(ValidatorRequiredWith)
    Q_DISABLE_COPY(ValidatorRequiredWith)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORREQUIREDWITH_H
