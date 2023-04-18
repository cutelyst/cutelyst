/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatoraccepted_p.h"

#include <QStringList>

using namespace Cutelyst;

ValidatorAccepted::ValidatorAccepted(const QString &field, const Cutelyst::ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorAcceptedPrivate(field, messages))
{
}

ValidatorAccepted::~ValidatorAccepted()
{
}

ValidatorReturnType ValidatorAccepted::validate(Cutelyst::Context *c, const Cutelyst::ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    if (Q_LIKELY(ValidatorAccepted::validate(value(params)))) {
        result.value.setValue<bool>(true);
    } else {
        result.errorMessage = validationError(c);
        result.value.setValue<bool>(false);
        qCDebug(C_VALIDATOR, "ValidatorAccepted: Validation failed for field %s at %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
    }

    return result;
}

bool ValidatorAccepted::validate(const QString &value)
{
    bool ret = true;
    static const QStringList l({QStringLiteral("yes"), QStringLiteral("on"), QStringLiteral("1"), QStringLiteral("true")});
    ret = l.contains(value, Qt::CaseInsensitive);
    return ret;
}

QString ValidatorAccepted::genericValidationError(Cutelyst::Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorAccepted", "Has to be accepted.");
    } else {
        //: %1 will be replaced by the field label
        error = c->translate("Cutelyst::ValidatorAccepted", "“%1” has to be accepted.");
    }
    return error;
}
