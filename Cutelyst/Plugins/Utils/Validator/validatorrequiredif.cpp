/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorrequiredif_p.h"

using namespace Cutelyst;

ValidatorRequiredIf::ValidatorRequiredIf(const QString &field, const QString &otherField, const QStringList &otherValues, const Cutelyst::ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorRequiredIfPrivate(field, otherField, otherValues, messages))
{
}

ValidatorRequiredIf::~ValidatorRequiredIf()
{
}

ValidatorReturnType ValidatorRequiredIf::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorRequiredIf);

    if (d->otherField.isEmpty() || d->otherValues.empty()) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR, "ValidatorRequiredIf: invalid validation data for field %s at %s::%s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
    } else {
        const QString v  = value(params);
        const QString ov = trimBefore() ? params.value(d->otherField).trimmed() : params.value(d->otherField);
        if (d->otherValues.contains(ov)) {
            if (v.isEmpty()) {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR, "ValidatorRequiredIf: Validation failed for field %s at %s::%s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
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

QString ValidatorRequiredIf::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorRequiredIf", "This is required.");
    } else {
        //: %1 will be replaced by the field label
        error = c->translate("Cutelyst::ValidatorRequiredIf", "The “%1” field is required.").arg(_label);
    }
    return error;
}
