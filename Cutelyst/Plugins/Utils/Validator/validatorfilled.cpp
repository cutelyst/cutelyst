/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorfilled_p.h"

using namespace Cutelyst;

ValidatorFilled::ValidatorFilled(const QString &field, const Cutelyst::ValidatorMessages &messages, const QString &defValKey)
    : ValidatorRule(*new ValidatorFilledPrivate(field, messages, defValKey))
{
}

ValidatorFilled::~ValidatorFilled()
{
}

ValidatorReturnType ValidatorFilled::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    if (params.contains(field())) {
        const QString v = value(params);
        if (!v.isEmpty()) {
            result.value.setValue(v);
        } else {
            result.errorMessage = validationError(c);
        }
    } else {
        defaultValue(c, &result, "ValidatorAfter");
    }

    return result;
}

QString ValidatorFilled::genericValidationError(Context *c, const QVariant &errorData) const
{
    Q_UNUSED(errorData)
    QString error;
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorFilled", "Must be filled.");
    } else {
        //: %1 will be replaced by the field label
        error = c->translate("Cutelyst::ValidatorFilled", "You must fill in the “%1” field.");
    }
    return error;
}
