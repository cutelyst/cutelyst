/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatordifferent_p.h"

using namespace Cutelyst;

ValidatorDifferent::ValidatorDifferent(const QString &field, const QString &other, const char *otherLabel, const Cutelyst::ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorDifferentPrivate(field, other, otherLabel, messages))
{
}

ValidatorDifferent::~ValidatorDifferent()
{
}

ValidatorReturnType ValidatorDifferent::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorDifferent);

    const QString v = value(params);
    const QString o = trimBefore() ? params.value(d->otherField).trimmed() : params.value(d->otherField);

    if (!v.isEmpty()) {
        if ((v == o)) {
            result.errorMessage = validationError(c);
            qCDebug(C_VALIDATOR, "ValidatorDifferent: Validation failed for value %s in field %s at %s::%s: the value in the %s field is not different.", qPrintable(v), qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), qPrintable(d->otherField));
        } else {
            result.value.setValue(v);
        }
    }

    return result;
}

QString ValidatorDifferent::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;

    Q_D(const ValidatorDifferent);

    Q_UNUSED(errorData);

    const QString _label      = label(c);
    const QString _otherLabel = d->otherLabel ? c->translate(d->translationContext.data(), d->otherLabel) : QString();

    if (_label.isEmpty()) {
        //: %1 will be replaced by the other field's label to compare with
        error = c->translate("Cutelyst::ValidatorDifferent", "Has to be different from the value in the “%1” field.").arg(!_otherLabel.isEmpty() ? _otherLabel : d->otherField);
    } else {
        //: %1 will be replaced by the field label, %2 will be replaced by the other field's label to compare with
        error = c->translate("Cutelyst::ValidatorDifferent", "The value in the “%1” field has to be different from the value in the “%2“ field.").arg(_label, !_otherLabel.isEmpty() ? _otherLabel : d->otherField);
    }

    return error;
}
