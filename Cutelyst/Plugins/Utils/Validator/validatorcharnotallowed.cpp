/*
 * Copyright (C) 2019 Matthias Fehring <mf@huessenbergnetz.de>
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

#include "validatorcharnotallowed_p.h"

using namespace Cutelyst;

ValidatorCharNotAllowed::ValidatorCharNotAllowed(const QString &field, const QString &forbiddenChars, const ValidatorMessages &messages, const QString &defValKey) :
    ValidatorRule(*new ValidatorCharNotAllowedPrivate(field, forbiddenChars, messages, defValKey))
{

}

ValidatorCharNotAllowed::~ValidatorCharNotAllowed()
{

}

bool ValidatorCharNotAllowed::validate(const QString &value, const QString &forbiddenChars, QChar *foundChar)
{
    bool valid = true;

    for (const QChar &forbiddenChar : forbiddenChars) {
        if (value.contains(forbiddenChar)) {
            valid = false;
            if (foundChar) {
                *foundChar = forbiddenChar;
            }
            break;
        }
    }

    return valid;
}

ValidatorReturnType ValidatorCharNotAllowed::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorCharNotAllowed);

    const QString v = value(params);
    if (!v.isEmpty()) {
        if (Q_LIKELY(!d->forbiddenChars.isEmpty())) {
            QChar foundChar;
            if (Q_LIKELY(ValidatorCharNotAllowed::validate(v, d->forbiddenChars, &foundChar))) {
                result.value.setValue(v);
            } else {
                result.errorMessage = validationError(c, foundChar);
            }
        } else {
            qCWarning(C_VALIDATOR) << "ValidatorCharNotAllowed: Empty validation data for field" << field() << "at" << c->controllerName() << "::" << c->actionName();
            result.errorMessage = validationDataError(c);
        }
    } else {
        defaultValue(c, &result, "ValidatorCharNotAllowed");
    }

    return result;
}

QString ValidatorCharNotAllowed::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    const QChar foundChar = errorData.toChar();
    Q_D(const ValidatorCharNotAllowed);
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorCharNotAllowed", "Must not contain the following characters: “%1”. But contains the following illegal character: “%2”.").arg(d->forbiddenChars, QString(foundChar));
    } else {
        error = c->translate("Cutelyst::ValidatorCharNotAllowed", "The text in the “%1“ field must not contain the following characters: “%2“. But contains the following illegal character: “%3”.").arg(_label, d->forbiddenChars, QString(foundChar));
    }

    return error;
}

QString ValidatorCharNotAllowed::genericValidationDataError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorCharNotAllowed", "The list of illegal characters for this field is empty.");
    } else {
        error = c->translate("Cutelyst::ValidatorCharNotAllowed", "The list of illegal characters for the “%1“ field is empty.").arg(_label);
    }
    return error;
}
