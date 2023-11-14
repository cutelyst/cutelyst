/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatornumeric_p.h"

using namespace Cutelyst;

ValidatorNumeric::ValidatorNumeric(const QString &field,
                                   const Cutelyst::ValidatorMessages &messages,
                                   const QString &defValKey)
    : ValidatorRule(*new ValidatorNumericPrivate(field, messages, defValKey))
{
}

ValidatorNumeric::~ValidatorNumeric() = default;

ValidatorReturnType ValidatorNumeric::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    const QString v = value(params);

    if (!v.isEmpty()) {
        bool ok       = false;
        const auto _v = v.toDouble(&ok);
        if (Q_LIKELY(ok)) {
            result.value.setValue(_v);
        } else {
            qCDebug(C_VALIDATOR).noquote().nospace()
                << debugString(c) << " Can not convert input value \"" << v
                << "\" into a numeric value";
            result.errorMessage = validationError(c);
        }
    } else {
        defaultValue(c, &result);
    }

    return result;
}

QString ValidatorNumeric::genericValidationError(Context *c, const QVariant &errorData) const
{
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        //% "Must be numeric, like 1, -2.5 or 3.454e3."
        return c->qtTrId("cutelyst-valnumeric-genvalerr");
    } else {
        //: %1 will be replaced by the field label
        //% "You have to enter a numeric value into the “%1” field, like 1, -2.5 or 3.454e3."
        return c->qtTrId("cutelyst-valnumeric-genvalerr-label").arg(_label);
    }
}
