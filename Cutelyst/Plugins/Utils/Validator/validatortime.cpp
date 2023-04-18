/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatortime_p.h"

#include <QTime>

using namespace Cutelyst;

ValidatorTime::ValidatorTime(const QString &field, const char *format, const Cutelyst::ValidatorMessages &messages, const QString &defValKey)
    : ValidatorRule(*new ValidatorTimePrivate(field, format, messages, defValKey))
{
}

ValidatorTime::~ValidatorTime()
{
}

ValidatorReturnType ValidatorTime::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorTime);

    const QString v = value(params);

    if (!v.isEmpty()) {
        const QTime time = d->extractTime(c, v, d->format);

        if (!time.isValid()) {
            result.errorMessage = validationError(c);
            qCDebug(C_VALIDATOR, "ValidatorTime: Validation failed for value \"%s\" in field %s at %s::%s: not a valid time", qPrintable(v), qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
        } else {
            result.value.setValue(time);
        }

    } else {
        defaultValue(c, &result, "ValidatorTime");
    }

    return result;
}

QString ValidatorTime::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;

    Q_D(const ValidatorTime);

    Q_UNUSED(errorData)

    const QString _label = label(c);

    if (_label.isEmpty()) {

        if (d->format) {
            //: %1 will be replaced by the erquired time format
            error = c->translate("Cutelyst::ValidatorTime", "Not a valid time according to the following date format: %1").arg(c->translate(d->translationContext.data(), d->format));
        } else {
            error = c->translate("Cutelyst::ValidatorTime", "Not a valid time.");
        }

    } else {

        if (d->format) {
            //: %1 will be replaced by the field label, %2 will be replaced by the required time format
            error = c->translate("Cutelyst::ValidatorTime", "The value in the “%1” field can not be parsed as time according to the following scheme: %2").arg(_label, c->translate(d->translationContext.data(), d->format));
        } else {
            //: %1 will be replaced by the field label
            error = c->translate("Cutelyst::ValidatorTime", "The value in the “%1” field can not be parsed as time.").arg(_label);
        }
    }

    return error;
}
