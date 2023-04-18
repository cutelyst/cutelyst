/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorrequiredwithout_p.h"

using namespace Cutelyst;

ValidatorRequiredWithout::ValidatorRequiredWithout(const QString &field, const QStringList &otherFields, const Cutelyst::ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorRequiredWithoutPrivate(field, otherFields, messages))
{
}

ValidatorRequiredWithout::~ValidatorRequiredWithout()
{
}

ValidatorReturnType ValidatorRequiredWithout::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorRequiredWithout);

    if (d->otherFields.isEmpty()) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR, "ValidatorRequiredWithout: invalid validation data for field %s at %s::%s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
    } else {

        bool otherMissing = false;

        const QStringList ofc = d->otherFields;

        for (const QString &other : ofc) {
            if (!params.contains(other)) {
                otherMissing = true;
                break;
            }
        }

        const QString v = value(params);

        if (otherMissing) {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            } else {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR, "ValidatorRequiredWithout: Validation failed for field %s at %s::%s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            }
        } else {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            }
        }
    }

    return result;
}

QString ValidatorRequiredWithout::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorRequiredWithout", "This is required.");
    } else {
        //: %1 will be replaced by the field label
        error = c->translate("Cutelyst::ValidatorRequiredWithout", "The “%1” field is required.").arg(_label);
    }
    return error;
}
