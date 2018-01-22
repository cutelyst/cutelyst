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

#include "validatordate_p.h"
#include <QDate>

using namespace Cutelyst;

ValidatorDate::ValidatorDate(const QString &field, const char *inputFormat, const Cutelyst::ValidatorMessages &messages, const QString &defValKey) :
    ValidatorRule(*new ValidatorDatePrivate(field, inputFormat, messages, defValKey))
{
}

ValidatorDate::~ValidatorDate()
{
}


ValidatorReturnType ValidatorDate::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorDate);

    const QString v = value(params);

    if (!v.isEmpty()) {
        const QDate date = d->extractDate(c, v, d->inputFormat);

        if (!date.isValid()) {
            result.errorMessage = validationError(c);
            qCDebug(C_VALIDATOR, "ValidatorDate: Validation failed for value \"%s\" in field %s in %s::%s: not a valid date.", qPrintable(v), qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
        } else {
            result.value.setValue<QDate>(date);
        }
    } else {
        defaultValue(c, &result, "ValidatorDate");
    }

    return result;
}

QString ValidatorDate::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;

    Q_D(const ValidatorDate);
    Q_UNUSED(errorData)

    const QString _label = label(c);

    if (_label.isEmpty()) {

        if (d->inputFormat) {
            error = c->translate("Cutelyst::ValidatorDate", "Not a valid date according to the following date format: %1").arg(c->translate(d->translationContext.data(), d->inputFormat));
        } else {
            error = c->translate("Cutelyst::ValidatorDate", "Not a valid date.");
        }

    } else {

        if (d->inputFormat) {
            error = c->translate("Cutelyst::ValidatorDate", "The value in the “%1” field can not be parsed as date according to the following scheme: %2").arg(_label, c->translate(d->translationContext.data(), d->inputFormat));
        } else {
            error = c->translate("Cutelyst::ValidatorDate", "The value in the “%1” field can not be parsed as date.").arg(_label);
        }
    }

    return error;
}
