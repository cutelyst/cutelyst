/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorregularexpression_p.h"

using namespace Cutelyst;

ValidatorRegularExpression::ValidatorRegularExpression(const QString &field,
                                                       const QRegularExpression &regex,
                                                       const ValidatorMessages &messages,
                                                       const QString &defValKey)
    : ValidatorRule(*new ValidatorRegularExpressionPrivate(field, regex, messages, defValKey))
{
}

ValidatorRegularExpression::~ValidatorRegularExpression() = default;

ValidatorReturnType ValidatorRegularExpression::validate(Context *c,
                                                         const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorRegularExpression);

    const QString v = value(params);

    if (d->regex.isValid()) {
        if (!v.isEmpty()) {
            if (v.contains(d->regex)) {
                result.value.setValue(v);
            } else {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR).noquote().nospace()
                    << debugString(c) << " value \"" << v << "\" does not match " << d->regex;
            }
        } else {
            defaultValue(c, &result);
        }
    } else {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR).noquote().nospace()
            << debugString(c) << " the regular expression is not valid: " << d->regex.errorString();
    }

    return result;
}

QString ValidatorRegularExpression::genericValidationError(Context *c,
                                                           const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorRegularExpression",
                             "Does not match the desired format.");
    } else {
        //: %1 will be replaced by the field label
        error = c->translate("Cutelyst::ValidatorRegularExpression",
                             "The “%1” field does not match the desired format.")
                    .arg(_label);
    }
    return error;
}
