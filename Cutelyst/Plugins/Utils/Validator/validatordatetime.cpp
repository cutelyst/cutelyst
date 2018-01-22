/*
 * Copyright (C) 2017-2018 Matthias Fehring <kontakt@buschmann23.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "validatordatetime_p.h"

#include <QDateTime>

using namespace Cutelyst;

ValidatorDateTime::ValidatorDateTime(const QString &field, const QString &timeZone, const char *inputFormat, const ValidatorMessages &messages, const QString &defValKey) :
    ValidatorRule(*new ValidatorDateTimePrivate(field, timeZone, inputFormat, messages, defValKey))
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
            result.value.setValue<QDateTime>(dt);
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
            error = c->translate("Cutelyst::ValidatorDateTime", "Not a valid date and time according to the following date format: %1").arg(c->translate(d->translationContext.data(), d->inputFormat));
        } else {
            error = c->translate("Cutelyst::ValidatorDateTime", "Not a valid date and time.");
        }

    } else {

        if (d->inputFormat) {
            error = c->translate("Cutelyst::ValidatorDateTime", "The value in the “%1” field can not be parsed as date and time according to the following date and time format: %2").arg(_label, c->translate(d->translationContext.data(), d->inputFormat));
        } else {
            error = c->translate("Cutelyst::ValidatorDateTime", "The value in the “%1” field can not be prased as date and time.").arg(_label);
        }
    }

    return error;
}
