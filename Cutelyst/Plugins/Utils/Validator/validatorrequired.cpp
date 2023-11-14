/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorrequired_p.h"

using namespace Cutelyst;

ValidatorRequired::ValidatorRequired(const QString &field,
                                     const Cutelyst::ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorRequiredPrivate(field, messages))
{
}

ValidatorRequired::~ValidatorRequired() = default;

ValidatorReturnType ValidatorRequired::validate(Cutelyst::Context *c,
                                                const Cutelyst::ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    if (!params.contains(field())) {
        qCDebug(C_VALIDATOR).noquote() << debugString(c) << "Field not found";
        result.errorMessage = validationError(c);
        return result;
    }

    const QString v = value(params);
    if (Q_LIKELY(!v.isEmpty())) {
        result.value.setValue(v);
    } else {
        qCDebug(C_VALIDATOR).noquote() << debugString(c) << "The field is not present or empty";
        result.errorMessage = validationError(c);
    }

    return result;
}

QString ValidatorRequired::genericValidationError(Cutelyst::Context *c,
                                                  const QVariant &errorData) const
{
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        //% "This field is required."
        return c->qtTrId("cutelyst-validator-genvalerr-req");
    } else {
        //: %1 will be replaced by the field label
        //% "The “%1” field is required."
        return c->qtTrId("cutelyst-validator-genvalerr-req-label").arg(_label);
    }
}
