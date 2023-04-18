/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorrequiredwithoutall_p.h"

using namespace Cutelyst;

ValidatorRequiredWithoutAll::ValidatorRequiredWithoutAll(const QString &field, const QStringList &otherFields, const Cutelyst::ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorRequiredWithoutAllPrivate(field, otherFields, messages))
{
}

ValidatorRequiredWithoutAll::~ValidatorRequiredWithoutAll()
{
}

ValidatorReturnType ValidatorRequiredWithoutAll::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorRequiredWithoutAll);

    if (d->otherFields.empty()) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR, "ValidatorRequiredWithoutAll: invalid validation data for field %s at %s::%s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
    } else {

        const QStringList ofc = d->otherFields;

        bool withoutAll = true;

        for (const QString &other : ofc) {
            if (params.contains(other)) {
                withoutAll = false;
                break;
            }
        }

        const QString v = value(params);

        if (withoutAll) {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            } else {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR, "ValidatorRequiredWithoutAll: Validation failed for field %s at %s::%s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            }
        } else {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            }
        }
    }

    return result;
}

QString ValidatorRequiredWithoutAll::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorRequiredWithoutAll", "This is required.");
    } else {
        //: %1 will be replaced by the field label
        error = c->translate("Cutelyst::ValidatorRequiredWithoutAll", "The “%1” field is required.").arg(_label);
    }
    return error;
}
