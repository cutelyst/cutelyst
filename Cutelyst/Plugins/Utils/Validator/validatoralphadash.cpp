﻿/*
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

#include "validatoralphadash_p.h"
#include <QRegularExpression>

using namespace Cutelyst;

ValidatorAlphaDash::ValidatorAlphaDash(const QString &field, bool asciiOnly, const ValidatorMessages &messages, const QString &defValKey) :
    ValidatorRule(*new ValidatorAlphaDashPrivate(field, asciiOnly, messages, defValKey))
{

}

ValidatorAlphaDash::~ValidatorAlphaDash()
{

}

ValidatorReturnType ValidatorAlphaDash::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorAlphaDash);

    const QString v = value(params);
    if (!v.isEmpty()) {
        if (Q_LIKELY(ValidatorAlphaDash::validate(v, d->asciiOnly))) {
            result.value.setValue<QString>(v);
        } else {
            qCDebug(C_VALIDATOR, "ValidatorAlphaDash: Validation failed for field %s at %s::%s: %s contains characters that are not allowed.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), qPrintable(v));
            result.errorMessage = validationError(c);
        }
    } else {
        defaultValue(c, &result, "ValidatorAlphaDash");
    }

    return result;
}

bool ValidatorAlphaDash::validate(const QString &value, bool asciiOnly)
{
    bool valid = true;
    if (asciiOnly) {
        for (const QChar &ch : value) {
            const ushort &uc = ch.unicode();
            if (!(((uc > 64) && (uc < 91)) || ((uc > 96) && (uc < 123)) || ((uc > 47) && (uc < 58)) || (uc == 45) || (uc == 95))) {
                valid = false;
                break;
            }
        }
    } else {
        valid = value.contains(QRegularExpression(QStringLiteral("^[\\pL\\pM\\pN_-]+$")));
    }
    return valid;
}

QString ValidatorAlphaDash::genericValidationError(Cutelyst::Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorAlphaDash", "Can only contain alpha-numeric characters, dashes and underscores.");
    } else {
        error = c->translate("Cutelyst::ValidatorAlphaDash", "The “%1” field can only contain alpha-numeric characters, as well as dashes and underscores, but nothing else.").arg(_label);
    }
    return error;
}
