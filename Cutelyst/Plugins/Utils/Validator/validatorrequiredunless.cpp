/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorrequiredunless_p.h"

using namespace Cutelyst;

ValidatorRequiredUnless::ValidatorRequiredUnless(const QString &field,
                                                 const QString &otherField,
                                                 const QStringList &otherValues,
                                                 const Cutelyst::ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorRequiredUnlessPrivate(field, otherField, otherValues, messages))
{
}

ValidatorRequiredUnless::~ValidatorRequiredUnless() = default;

ValidatorReturnType ValidatorRequiredUnless::validate(Context *c,
                                                      const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorRequiredUnless);

    if (d->otherField.isEmpty() || d->otherValues.empty()) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR).noquote() << "Invalid validation data";
    } else {
        const QString v = value(params);
        const QString ov =
            trimBefore() ? params.value(d->otherField).trimmed() : params.value(d->otherField);
        if (!d->otherValues.contains(ov)) {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            } else {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR).noquote().nospace()
                    << debugString(c) << " The field is not present or empty but \""
                    << d->otherField << "\" not contains " << ov;
            }
        } else {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            }
        }
    }

    return result;
}

QString ValidatorRequiredUnless::genericValidationError(Context *c, const QVariant &errorData) const
{
    // translation strings are defined in ValidatorRequired
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        return c->qtTrId("cutelyst-validator-genvalerr-req");
    } else {
        return c->qtTrId("cutelyst-validator-genvalerr-req-label").arg(_label);
    }
}
