﻿/*
 * SPDX-FileCopyrightText: (C) 2017-2025 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorrequiredwithout_p.h"

using namespace Cutelyst;

ValidatorRequiredWithout::ValidatorRequiredWithout(const QString &field,
                                                   const QStringList &otherFields,
                                                   const Cutelyst::ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorRequiredWithoutPrivate(field, otherFields, messages))
{
}

ValidatorRequiredWithout::~ValidatorRequiredWithout() = default;

ValidatorReturnType ValidatorRequiredWithout::validate(Context *c,
                                                       const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorRequiredWithout);

    if (d->otherFields.isEmpty()) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR).noquote() << "Invalid validation data";
    } else {
        const QString v = value(params);

        auto it = std::ranges::find_if(
            d->otherFields, [params](const QString &other) { return !params.contains(other); });

        bool otherMissing = (it != d->otherFields.end());
        if (otherMissing) {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            } else {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR).noquote().nospace()
                    << debugString(c) << " The field is not present or empty but the field \""
                    << *it << "\" is not present";
            }
        } else {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            }
        }
    }

    return result;
}

void ValidatorRequiredWithout::validateCb(Context *c,
                                          const ParamsMultiMap &params,
                                          ValidatorRtFn cb) const
{
    cb(validate(c, params));
}

QString ValidatorRequiredWithout::genericValidationError(Context *c,
                                                         const QVariant &errorData) const
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
