/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORREGEX_H
#define CUTELYSTVALIDATORREGEX_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

#include <QRegularExpression>

namespace Cutelyst {

class ValidatorRegularExpressionPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorRegularExpression validatorregularexpression.h <Cutelyst/Plugins/Utils/validatorregularexpression.h>
 * \brief The field under validation must match the given regular expression.
 *
 * Checks if the \a regex matches the content of the \a field.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation.
 * If the \a field's value is empty or if the \a field is missing in the input data, the validation will succeed without
 * performing the validation itself. Use one of the \link ValidatorRequired required validators \endlink to require the
 * field to be present and not empty.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorCharNotAllowed
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorRegularExpression : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new regex validator.
     * \param field         Name of the input field to validate.
     * \param regex         The regular expression to check against.
     * \param messages      Custom error message if validation fails.
     * \param defValKey     \link Context::stash() Stash \endlink key containing a default value if input field is empty. This value will \b NOT be validated.
     */
    ValidatorRegularExpression(const QString &field, const QRegularExpression &regex, const ValidatorMessages &messages = ValidatorMessages(), const QString &defValKey = QString());

    /*!
     * \brief Deconstructs the regex validator.
     */
    ~ValidatorRegularExpression() override;

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
    Q_DECLARE_PRIVATE(ValidatorRegularExpression)
    Q_DISABLE_COPY(ValidatorRegularExpression)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORREGEX_H
