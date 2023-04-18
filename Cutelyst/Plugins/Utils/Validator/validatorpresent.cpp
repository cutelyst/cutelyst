/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorpresent_p.h"

using namespace Cutelyst;

ValidatorPresent::ValidatorPresent(const QString &field, const Cutelyst::ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorPresentPrivate(field, messages))
{
}

ValidatorPresent::~ValidatorPresent()
{
}

ValidatorReturnType ValidatorPresent::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    if (!params.contains(field())) {
        result.errorMessage = validationError(c);
        qCDebug(C_VALIDATOR, "ValidatorPresent: Validation failed for field %s at %s::%s: field was not found in the input data", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
    } else {
        result.value.setValue<QString>(value(params));
    }

    return result;
}

QString ValidatorPresent::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorPresent", "Has to be present in input data.");
    } else {
        //: %1 will be replaced by the field label
        error = c->translate("Cutelyst::ValidatorPresent", "The “%1” field was not found in the input data.").arg(_label);
    }
    return error;
}
