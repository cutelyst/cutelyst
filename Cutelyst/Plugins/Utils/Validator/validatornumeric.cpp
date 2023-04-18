/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatornumeric_p.h"

using namespace Cutelyst;

ValidatorNumeric::ValidatorNumeric(const QString &field, const Cutelyst::ValidatorMessages &messages, const QString &defValKey)
    : ValidatorRule(*new ValidatorNumericPrivate(field, messages, defValKey))
{
}

ValidatorNumeric::~ValidatorNumeric()
{
}

ValidatorReturnType ValidatorNumeric::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    const QString v = value(params);

    if (!v.isEmpty()) {
        bool ok         = false;
        const double _v = v.toDouble(&ok);
        if (Q_LIKELY(ok)) {
            result.value.setValue(_v);
        } else {
            qCDebug(C_VALIDATOR, "ValidatorNumeric: Validation failed for field %s at %s::%s: can not convert input value into a numeric value.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            result.errorMessage = validationError(c);
        }
    } else {
        defaultValue(c, &result, "ValidatorNumeric");
    }

    return result;
}

QString ValidatorNumeric::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorNumeric", "Must be numeric, like 1, -2.5 or 3.454e3.");
    } else {
        //: %1 will be replaced by the field label
        error = c->translate("Cutelyst::ValidatorNumeric", "You have to enter a numeric value into the “%1” field, like 1, -2.5 or 3.454e3.").arg(_label);
    }
    return error;
}
