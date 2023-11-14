/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorpresent_p.h"

using namespace Cutelyst;

ValidatorPresent::ValidatorPresent(const QString &field,
                                   const Cutelyst::ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorPresentPrivate(field, messages))
{
}

ValidatorPresent::~ValidatorPresent() = default;

ValidatorReturnType ValidatorPresent::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    if (!params.contains(field())) {
        result.errorMessage = validationError(c);
        qCDebug(C_VALIDATOR).noquote() << debugString(c) << "Field not found";
    } else {
        result.value.setValue<QString>(value(params));
    }

    return result;
}

QString ValidatorPresent::genericValidationError(Context *c, const QVariant &errorData) const
{
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        //% "Has to be present in input data."
        return c->qtTrId("cutelyst-vapresent-genvalerr");
    } else {
        //: %1 will be replaced by the field label
        //% "The “%1” field was not found in the input data."
        return c->qtTrId("cutelyst-vapresent-genvalerr-label").arg(_label);
    }
}
