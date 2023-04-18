/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorboolean_p.h"

#include <QStringList>

using namespace Cutelyst;

ValidatorBoolean::ValidatorBoolean(const QString &field, const ValidatorMessages &messages, const QString &defValKey)
    : ValidatorRule(*new ValidatorBooleanPrivate(field, messages, defValKey))
{
}

ValidatorBoolean::~ValidatorBoolean()
{
}

ValidatorReturnType ValidatorBoolean::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    const QString v = value(params);

    if (!v.isEmpty()) {
        static const QStringList lt({QStringLiteral("1"), QStringLiteral("true"), QStringLiteral("on")});
        static const QStringList lf({QStringLiteral("0"), QStringLiteral("false"), QStringLiteral("off")});
        if (lt.contains(v, Qt::CaseInsensitive)) {
            result.value.setValue<bool>(true);
        } else if (lf.contains(v, Qt::CaseInsensitive)) {
            result.value.setValue<bool>(false);
        } else {
            result.errorMessage = validationError(c);
            qCDebug(C_VALIDATOR, "ValidatorBoolean: The value %s of field %s in %s::%s can not be interpreted as boolean.", qPrintable(v), qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
        }
    } else {
        defaultValue(c, &result, "ValidatorBoolean");
    }

    return result;
}

QString ValidatorBoolean::genericValidationError(Cutelyst::Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorBoolean", "Can not be interpreted as boolean value.");
    } else {
        //: %1 will be replaced by the field label
        error = c->translate("Cutelyst::ValidatorBoolean", "The value in the “%1” field can not be interpreted as a boolean value.").arg(_label);
    }
    return error;
}
