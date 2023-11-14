/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatordate_p.h"

#include <QDate>

using namespace Cutelyst;

ValidatorDate::ValidatorDate(const QString &field,
                             const char *inputFormat,
                             const Cutelyst::ValidatorMessages &messages,
                             const QString &defValKey)
    : ValidatorRule(*new ValidatorDatePrivate(field, inputFormat, messages, defValKey))
{
}

ValidatorDate::~ValidatorDate() = default;

ValidatorReturnType ValidatorDate::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorDate);

    const QString v = value(params);

    if (!v.isEmpty()) {
        const QDate date = d->extractDate(c, v, d->inputFormat);

        if (!date.isValid()) {
            result.errorMessage = validationError(c);
            qCDebug(C_VALIDATOR).noquote().nospace()
                << debugString(c) << " \"" << v << "\" is not a valid date";
        } else {
            result.value.setValue(date);
        }
    } else {
        defaultValue(c, &result);
    }

    return result;
}

QString ValidatorDate::genericValidationError(Context *c, const QVariant &errorData) const
{
    Q_D(const ValidatorDate);
    Q_UNUSED(errorData)

    const QString _label = label(c);

    if (d->inputFormat) {
        const QString inputFormatTranslated =
            d->translationContext ? c->translate(d->translationContext, d->inputFormat)
                                  : c->qtTrId(d->inputFormat);
        if (_label.isEmpty()) {
            //: %1 will be replaced by the required date format
            //% "Not a valid date according to the following format: %1"
            return c->qtTrId("cutelyst-valdate-genvalerr-format").arg(inputFormatTranslated);
        } else {
            //: %1 will be replaced by the field label, %2 will be replaced
            //: by the required date format
            //% "The value in the “%1” field can not be parsed as date according "
            //% "to the following format: %2"
            return c->qtTrId("cutelyst-valdate-genvalerr-format-label")
                .arg(_label, inputFormatTranslated);
        }
    } else {
        if (_label.isEmpty()) {
            //% "Not a valid date."
            return c->qtTrId("cutelyst-valdate-genvalerr");
        } else {
            //: %1 will be replaced by the field label
            //% "The value in the “%1” field can not be parsed as date."
            return c->qtTrId("cutelyst-valdate-genvalerr-label").arg(_label);
        }
    }
}
