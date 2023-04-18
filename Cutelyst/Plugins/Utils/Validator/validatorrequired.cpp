/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorrequired_p.h"

using namespace Cutelyst;

ValidatorRequired::ValidatorRequired(const QString &field, const Cutelyst::ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorRequiredPrivate(field, messages))
{
}

ValidatorRequired::~ValidatorRequired()
{
}

ValidatorReturnType ValidatorRequired::validate(Cutelyst::Context *c, const Cutelyst::ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    const QString v = value(params);
    if (Q_LIKELY(!v.isEmpty())) {
        result.value.setValue(v);
    } else {
        result.errorMessage = validationError(c);
    }

    return result;
}

QString ValidatorRequired::genericValidationError(Cutelyst::Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorRequired", "This is required.");
    } else {
        //: %1 will be replaced by the field label
        error = c->translate("Cutelyst::ValidatorRequired", "The “%1” field is required.").arg(_label);
    }
    return error;
}
