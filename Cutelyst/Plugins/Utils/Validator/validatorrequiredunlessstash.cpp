/*
 * SPDX-FileCopyrightText: (C) 2018-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorrequiredunlessstash_p.h"

using namespace Cutelyst;

ValidatorRequiredUnlessStash::ValidatorRequiredUnlessStash(const QString &field,
                                                           const QString &stashKey,
                                                           const QVariantList &stashValues,
                                                           const ValidatorMessages &messages)
    : ValidatorRule(
          *new ValidatorRequiredUnlessStashPrivate(field, stashKey, stashValues, messages))
{
}

ValidatorRequiredUnlessStash::~ValidatorRequiredUnlessStash() = default;

ValidatorReturnType ValidatorRequiredUnlessStash::validate(Context *c,
                                                           const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorRequiredUnlessStash);

    if (d->stashKey.isEmpty() || d->stashValues.empty()) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR).noquote() << "Invalid validation data";
    } else {
        const QString v   = value(params);
        const QVariant sv = c->stash(d->stashKey);
        if (!d->stashValues.contains(sv)) {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            } else {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR).noquote().nospace()
                    << debugString(c) << " The field is not present or empty but stash key \""
                    << d->stashKey << "\" not contains " << sv;
            }
        } else {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            }
        }
    }

    return result;
}

QString ValidatorRequiredUnlessStash::genericValidationError(Context *c,
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
