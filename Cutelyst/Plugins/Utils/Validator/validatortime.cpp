/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatortime_p.h"

#include <QTime>

using namespace Cutelyst;

ValidatorTime::ValidatorTime(const QString &field,
                             const char *format,
                             const Cutelyst::ValidatorMessages &messages,
                             const QString &defValKey)
    : ValidatorRule(*new ValidatorTimePrivate(field, format, messages, defValKey))
{
}

ValidatorTime::~ValidatorTime() = default;

ValidatorReturnType ValidatorTime::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorTime);

    const QString v = value(params);

    if (!v.isEmpty()) {
        const QTime time = d->extractTime(c, v, d->format);

        if (!time.isValid()) {
            result.errorMessage = validationError(c);
            qCDebug(C_VALIDATOR).noquote().nospace()
                << debugString(c) << " \"" << v << "\" is not a valid time";
        } else {
            result.value.setValue(time);
        }

    } else {
        defaultValue(c, &result);
    }

    return result;
}

QString ValidatorTime::genericValidationError(Context *c, const QVariant &errorData) const
{
    Q_D(const ValidatorTime);
    Q_UNUSED(errorData)
    const QString _label = label(c);

    if (d->format) {
        const QString _formatTranslated = d->translationContext
                                              ? c->translate(d->translationContext, d->format)
                                              : c->qtTrId(d->format);
        if (_label.isEmpty()) {
            //% "Not a valid time according to the following format: %1"
            return c->qtTrId("cutelyst-valtime-genvalerr-format").arg(_formatTranslated);
        } else {
            //% "The value in the “%1” field can not be parsed as time according "
            //% "to the following format: %2"
            return c->qtTrId("cutelyst-valtime-genvalerr-format-label")
                .arg(_label, _formatTranslated);
        }
    } else {
        if (_label.isEmpty()) {
            //% "Not a valid time."
            return c->qtTrId("cutelyst-valtime-genvalerr");
        } else {
            //% "The value in the “%1” field can not be parsed as time."
            return c->qtTrId("cutelyst-valtime-genvalerr-label").arg(_label);
        }
    }
}
