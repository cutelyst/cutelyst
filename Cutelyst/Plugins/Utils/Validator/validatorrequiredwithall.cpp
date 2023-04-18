/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorrequiredwithall_p.h"

using namespace Cutelyst;

ValidatorRequiredWithAll::ValidatorRequiredWithAll(const QString &field, const QStringList &otherFields, const ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorRequiredWithAllPrivate(field, otherFields, messages))
{
}

ValidatorRequiredWithAll::~ValidatorRequiredWithAll()
{
}

ValidatorReturnType ValidatorRequiredWithAll::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorRequiredWithAll);

    if (d->otherFields.empty()) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR, "ValidatorRequiredWithAll: invalid validation data for field %s at %s::%s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
    } else {

        bool containsAll = true;

        const QStringList ofc = d->otherFields;

        for (const QString &other : ofc) {
            if (!params.contains(other)) {
                containsAll = false;
                break;
            }
        }

        const QString v = value(params);

        if (containsAll) {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            } else {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR, "ValidatorRequiredWithAll: Validation failed for field %s at %s::%s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            }
        } else {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            }
        }
    }

    return result;
}

QString ValidatorRequiredWithAll::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    const QString _label = label(c);
    Q_UNUSED(errorData)
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorRequiredWithAll", "This is required.");
    } else {
        //: %1 will be replaced by the field label
        error = c->translate("Cutelyst::ValidatorRequiredWithAll", "The “%1” field is required.").arg(_label);
    }
    return error;
}
