/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorsame_p.h"

using namespace Cutelyst;

ValidatorSame::ValidatorSame(const QString &field, const QString &otherField, const char *otherLabel, const Cutelyst::ValidatorMessages &messages, const QString &defValKey)
    : ValidatorRule(*new ValidatorSamePrivate(field, otherField, otherLabel, messages, defValKey))
{
}

ValidatorSame::~ValidatorSame()
{
}

ValidatorReturnType ValidatorSame::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorSame);

    const QString v = value(params);

    if (!v.isEmpty()) {
        const QString ov = trimBefore() ? params.value(d->otherField).trimmed() : params.value(d->otherField);
        if (v != ov) {
            result.errorMessage = validationError(c);
            qCDebug(C_VALIDATOR, "ValidatorSame: Validation failed for field %s at %s::%s: value is not the same as in the field %s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), qPrintable(d->otherField));
        } else {
            result.value.setValue(v);
        }
    } else {
        defaultValue(c, &result, "ValidatorSame");
    }

    return result;
}

QString ValidatorSame::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;

    Q_D(const ValidatorSame);
    Q_UNUSED(errorData)
    const QString _label = label(c);
    QString _olabel;
    if (d->otherLabel) {
        _olabel = d->translationContext.size() ? c->translate(d->translationContext.data(), d->otherLabel) : QString::fromUtf8(d->otherLabel);
    } else {
        _olabel = d->otherField;
    }

    if (_label.isEmpty()) {
        //: %1 will be replaced by the label of the other field
        error = c->translate("Cutelyst::ValidatorSame", "Must be the same as in the “%1” field.").arg(_olabel);
    } else {
        //: %1 will be replaced by the field label, %2 will be replaced by the label of the other field
        error = c->translate("Cutelyst::ValidatorSame", "The “%1” field must have the same value as the “%2” field.").arg(_label, _olabel);
    }

    return error;
}
