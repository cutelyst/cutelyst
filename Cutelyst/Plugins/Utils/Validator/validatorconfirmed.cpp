/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorconfirmed_p.h"

using namespace Cutelyst;

ValidatorConfirmed::ValidatorConfirmed(const QString &field, const ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorConfirmedPrivate(field, messages))
{
}

ValidatorConfirmed::~ValidatorConfirmed()
{
}

ValidatorReturnType ValidatorConfirmed::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    const QString v = value(params);

    if (!v.isEmpty()) {
        const QString ofn = field() + QLatin1String("_confirmation");
        QString ofv       = params.value(ofn);

        if (trimBefore()) {
            ofv = ofv.trimmed();
        }

        if (Q_UNLIKELY(v != ofv)) {
            result.errorMessage = validationError(c);
            qCDebug(C_VALIDATOR, "ValidatorConfirmed: Failed to confirm the value in the field %s in %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
        } else {
            result.value.setValue(v);
        }
    }

    return result;
}

QString ValidatorConfirmed::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorConfirmed", "Confirmation failed.");
    } else {
        //: %1 will be replaced by the field label
        error = c->translate("Cutelyst::ValidatorConfirmed", "The value in the “%1“ field has not been confirmed.").arg(_label);
    }
    return error;
}
