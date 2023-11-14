/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatordatetime_p.h"

#include <QDateTime>

using namespace Cutelyst;

ValidatorDateTime::ValidatorDateTime(const QString &field,
                                     const QString &timeZone,
                                     const char *inputFormat,
                                     const ValidatorMessages &messages,
                                     const QString &defValKey)
    : ValidatorRule(
          *new ValidatorDateTimePrivate(field, timeZone, inputFormat, messages, defValKey))
{
}

ValidatorDateTime::~ValidatorDateTime() = default;

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
            qCDebug(C_VALIDATOR).noquote().nospace()
                << debugString(c) << " \"" << v << "\" is not a valid datetime";
        } else {
            result.value.setValue(dt);
        }

    } else {
        defaultValue(c, &result);
    }

    return result;
}

QString ValidatorDateTime::genericValidationError(Context *c, const QVariant &errorData) const
{
    Q_D(const ValidatorDateTime);
    Q_UNUSED(errorData)

    const QString _label = label(c);

    if (d->inputFormat) {
        const QString inputFormatTranslated =
            d->translationContext ? c->translate(d->translationContext, d->inputFormat)
                                  : c->qtTrId(d->inputFormat);
        if (_label.isEmpty()) {
            //: %1 will be replaced by the required date and time format
            //% "Not a valid date and time according to the following format: %1"
            return c->qtTrId("cutelyst-valdatetime-genvalerr-format").arg(inputFormatTranslated);
        } else {
            //: %1 will be replaced by the field label, %2 will be replaced by
            //: the required datetime format
            //% "The value in the “%1” field can not be parsed as date and time "
            //% "according to the following date and time format: %2"
            return c->qtTrId("cutelyst-valdatetime-genvalerr-format-label")
                .arg(_label, inputFormatTranslated);
        }
    } else {
        if (_label.isEmpty()) {
            //% "Not a valid date and time."
            return c->qtTrId("cutelyst-valdatetime-genvalerr");
        } else {
            //: %1 will be replaced by the field label
            //% "The value in the “%1” field can not be parsed as date and time."
            return c->qtTrId("cutelyst-valdatetime-genvalerr-label").arg(_label);
        }
    }
}
