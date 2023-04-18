/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORREQUIREDWITHOUT_H
#define CUTELYSTVALIDATORREQUIREDWITHOUT_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

#include <QStringList>

namespace Cutelyst {

class ValidatorRequiredWithoutPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorRequiredWithout validatorrequiredwithout.h <Cutelyst/Plugins/Utils/validatorrequiredwithout.h>
 * \brief The field under validation must be present and not empty only when any of the other specified fields are not present.
 *
 * If \b any of the fields in the \a otherFields list is \b not part of the input parameters, the \a field under validation must be present and not empty.
 * For the other fields it will only be checked if they are not present in the input parameters, not their content.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation. So, fields that only contain
 * whitespaces will be treated as empty.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorRequired, ValidatorRequiredIf, ValidatorRequiredUnless, ValidatorRequiredWith, ValidatorRequiredWithAll, ValidatorRequiredWithoutAll
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorRequiredWithout : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new required with validator.
     * \param field         Name of the input field to validate.
     * \param otherFields   List of other fields from which one has to be missing in the input to require the field.
     * \param messages      Custom error messages if validation fails.
     */
    ValidatorRequiredWithout(const QString &field, const QStringList &otherFields, const ValidatorMessages &messages = ValidatorMessages());

    /*!
     * \brief Deconstructs the required with validator.
     */
    ~ValidatorRequiredWithout() override;

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
    Q_DECLARE_PRIVATE(ValidatorRequiredWithout)
    Q_DISABLE_COPY(ValidatorRequiredWithout)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORREQUIREDWITHOUT_H
