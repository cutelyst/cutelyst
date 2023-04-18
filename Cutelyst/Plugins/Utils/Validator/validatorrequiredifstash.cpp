/*
 * SPDX-FileCopyrightText: (C) 2018-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorrequiredifstash_p.h"

using namespace Cutelyst;

ValidatorRequiredIfStash::ValidatorRequiredIfStash(const QString &field, const QString &stashKey, const QVariantList &stashValues, const ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorRequiredIfStashPrivate(field, stashKey, stashValues, messages))
{
}

ValidatorRequiredIfStash::~ValidatorRequiredIfStash()
{
}

ValidatorReturnType ValidatorRequiredIfStash::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorRequiredIfStash);

    if (d->stashKey.isEmpty() || d->stashValues.empty()) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR, "ValidatorRequiredIfStash: invalid validation data for field %s at %s::%s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
    } else {
        const QString v   = value(params);
        const QVariant sv = c->stash(d->stashKey);
        if (d->stashValues.contains(sv)) {
            if (v.isEmpty()) {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR, "ValidatorRequiredIfStash: Validation failed for field %s at %s::%s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            } else {
                result.value.setValue(v);
            }
        } else {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            }
        }
    }

    return result;
}

QString ValidatorRequiredIfStash::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorRequiredIfStash", "This is required.");
    } else {
        //: %1 will be replaced by the field label
        error = c->translate("Cutelyst::ValidatorRequiredIfStash", "The “%1” field is required.").arg(_label);
    }
    return error;
}
