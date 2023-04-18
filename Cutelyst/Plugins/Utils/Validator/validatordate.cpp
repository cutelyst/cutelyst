/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatordate_p.h"

#include <QDate>

using namespace Cutelyst;

ValidatorDate::ValidatorDate(const QString &field, const char *inputFormat, const Cutelyst::ValidatorMessages &messages, const QString &defValKey)
    : ValidatorRule(*new ValidatorDatePrivate(field, inputFormat, messages, defValKey))
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
            result.value.setValue(date);
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
            //: %1 will be replaced by the date format
            error = c->translate("Cutelyst::ValidatorDate", "Not a valid date according to the following date format: %1").arg(c->translate(d->translationContext.data(), d->inputFormat));
        } else {
            error = c->translate("Cutelyst::ValidatorDate", "Not a valid date.");
        }

    } else {

        if (d->inputFormat) {
            //: %1 will be replaced by the field label, %2 will be replaced by the date format
            error = c->translate("Cutelyst::ValidatorDate", "The value in the “%1” field can not be parsed as date according to the following scheme: %2").arg(_label, c->translate(d->translationContext.data(), d->inputFormat));
        } else {
            //: %1 will be replaced by the field label
            error = c->translate("Cutelyst::ValidatorDate", "The value in the “%1” field can not be parsed as date.").arg(_label);
        }
    }

    return error;
}
