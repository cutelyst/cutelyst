/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorrequiredunless_p.h"

using namespace Cutelyst;

ValidatorRequiredUnless::ValidatorRequiredUnless(const QString &field, const QString &otherField, const QStringList &otherValues, const Cutelyst::ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorRequiredUnlessPrivate(field, otherField, otherValues, messages))
{
}

ValidatorRequiredUnless::~ValidatorRequiredUnless()
{
}

ValidatorReturnType ValidatorRequiredUnless::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorRequiredUnless);

    if (d->otherField.isEmpty() || d->otherValues.empty()) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR, "ValidatorRequiredUnless: invalid validation data for field %s at %s::%s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
    } else {
        const QString v  = value(params);
        const QString ov = trimBefore() ? params.value(d->otherField).trimmed() : params.value(d->otherField);
        if (!d->otherValues.contains(ov)) {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            } else {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR, "ValidatorRequiredUnless: Validation failed for field %s at %s::%s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
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
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorRequiredUnless", "This is required.");
    } else {
        //: %1 will be replaced by the field label
        error = c->translate("Cutelyst::ValidatorRequiredUnless", "The “%1” field is required.").arg(_label);
    }
    return error;
}
