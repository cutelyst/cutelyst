/*
 * SPDX-FileCopyrightText: (C) 2018-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorrequiredunlessstash_p.h"

using namespace Cutelyst;

ValidatorRequiredUnlessStash::ValidatorRequiredUnlessStash(const QString &field, const QString &stashKey, const QVariantList &stashValues, const ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorRequiredUnlessStashPrivate(field, stashKey, stashValues, messages))
{
}

ValidatorRequiredUnlessStash::~ValidatorRequiredUnlessStash()
{
}

ValidatorReturnType ValidatorRequiredUnlessStash::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorRequiredUnlessStash);

    if (d->stashKey.isEmpty() || d->stashValues.empty()) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR, "ValidatorRequiredUnlessStash: invalid validation data for field %s at %s::%s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
    } else {
        const QString v   = value(params);
        const QVariant sv = c->stash(d->stashKey);
        if (!d->stashValues.contains(sv)) {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            } else {
                result.errorMessage = validationError(c);
            }
        } else {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            }
        }
    }

    return result;
}

QString ValidatorRequiredUnlessStash::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorRequiredUnlessStash", "This is required.");
    } else {
        //: %1 will be replaced by the field label
        error = c->translate("Cutelyst::ValidatorRequiredUnlessStash", "The “%1” field is required.").arg(_label);
    }
    return error;
}
