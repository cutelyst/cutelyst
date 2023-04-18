/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorrequiredwith_p.h"

using namespace Cutelyst;

ValidatorRequiredWith::ValidatorRequiredWith(const QString &field, const QStringList &otherFields, const Cutelyst::ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorRequiredWithPrivate(field, otherFields, messages))
{
}

ValidatorRequiredWith::~ValidatorRequiredWith()
{
}

ValidatorReturnType ValidatorRequiredWith::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorRequiredWith);

    if (d->otherFields.empty()) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR, "ValidatorRequiredWith: invalid validation data for field %s at %s::%s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
    } else {
        bool containsOther = false;
        const QString v    = value(params);

        const QStringList ofc = d->otherFields;

        for (const QString &other : ofc) {
            if (params.contains(other)) {
                containsOther = true;
                break;
            }
        }

        if (containsOther) {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            } else {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR, "ValidatorRequiredWith: Validation failed for field %s at %s::%s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            }
        } else {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            }
        }
    }

    return result;
}

QString ValidatorRequiredWith::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorRequiredWith", "This is required.");
    } else {
        //: %1 will be replaced by the field label
        error = c->translate("Cutelyst::ValidatorRequiredWith", "The “%1” field is required.").arg(_label);
    }
    return error;
}
