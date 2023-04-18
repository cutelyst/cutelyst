/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatordatetime_p.h"

#include <QDateTime>

using namespace Cutelyst;

ValidatorDateTime::ValidatorDateTime(const QString &field, const QString &timeZone, const char *inputFormat, const ValidatorMessages &messages, const QString &defValKey)
    : ValidatorRule(*new ValidatorDateTimePrivate(field, timeZone, inputFormat, messages, defValKey))
{
}

ValidatorDateTime::~ValidatorDateTime()
{
}

ValidatorReturnType ValidatorDateTime::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorDateTime);

    const QString v = value(params);

    if (!v.isEmpty()) {
        const QTimeZone tz = d->extractTimeZone(c, params, d->timeZone);
        const QDateTime dt = d->extractDateTime(c, v, d->inputFormat, tz);

        if (!dt.isValid()) {
            result.errorMessage = validationError(c);
            qCDebug(C_VALIDATOR, "ValidatorDateTime: Validation failed for value \"%s\" in field %s in %s::%s: not a valid date and time.", qPrintable(v), qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
        } else {
            result.value.setValue(dt);
        }

    } else {
        defaultValue(c, &result, "ValidatorDateTime");
    }

    return result;
}

QString ValidatorDateTime::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;

    Q_D(const ValidatorDateTime);
    Q_UNUSED(errorData)

    const QString _label = label(c);

    if (_label.isEmpty()) {

        if (d->inputFormat) {
            //: %1 will be replaced by the datetime format
            error = c->translate("Cutelyst::ValidatorDateTime", "Not a valid date and time according to the following format: %1").arg(c->translate(d->translationContext.data(), d->inputFormat));
        } else {
            error = c->translate("Cutelyst::ValidatorDateTime", "Not a valid date and time.");
        }

    } else {

        if (d->inputFormat) {
            //: %1 will be replaced by the field label, %2 will be replaced by the datetime format
            error = c->translate("Cutelyst::ValidatorDateTime", "The value in the “%1” field can not be parsed as date and time according to the following date and time format: %2").arg(_label, c->translate(d->translationContext.data(), d->inputFormat));
        } else {
            //: %1 will be replaced by the field label
            error = c->translate("Cutelyst::ValidatorDateTime", "The value in the “%1” field can not be parsed as date and time.").arg(_label);
        }
    }

    return error;
}
