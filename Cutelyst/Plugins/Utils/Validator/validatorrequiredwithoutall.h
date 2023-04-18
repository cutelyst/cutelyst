/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORREQUIREDWITHOUTALL_H
#define CUTELYSTVALIDATORREQUIREDWITHOUTALL_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

#include <QStringList>

namespace Cutelyst {

class ValidatorRequiredWithoutAllPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorRequiredWithoutAll validatorrequiredwithoutall.h <Cutelyst/Plugins/Utils/validatorrequiredwithoutall.h>
 * \brief The field under validation must be present and not empty only when all of the other specified fields are not present.
 *
 * If \b all of the fields specified in the \a otherFields list are \b not present in the input data, the \a field under validation
 * must be present and not empty. For the other fields it will only be checked if they are not part of the input data, not their content.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation. So, fields that only contain
 * whitespaces will be treated as empty.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorRequired, ValidatorRequiredIf, ValidatorRequiredUnless, ValidatorRequiredWith, ValidatorRequiredWithAll, ValidatorRequiredWithout
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorRequiredWithoutAll : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new required without all validator.
     * \param field         Name of the input field to validate.
     * \param otherFields   List of field names that are not allowed to be present to require the field.
     * \param messages      Custom error messages if validation fails.
     */
    ValidatorRequiredWithoutAll(const QString &field, const QStringList &otherFields, const ValidatorMessages &messages = ValidatorMessages());

    /*!
     * \brief Deconstructs the required without all validator.
     */
    ~ValidatorRequiredWithoutAll() override;

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
    Q_DECLARE_PRIVATE(ValidatorRequiredWithoutAll)
    Q_DISABLE_COPY(ValidatorRequiredWithoutAll)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORREQUIREDWITHOUTALL_H
