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

#include "validatordigits_p.h"

using namespace Cutelyst;

ValidatorDigits::ValidatorDigits(const QString &field, const QVariant &length, const Cutelyst::ValidatorMessages &messages, const QString &defValKey) :
    ValidatorRule(*new ValidatorDigitsPrivate(field, length, messages, defValKey))
{
}

ValidatorDigits::~ValidatorDigits()
{
}

ValidatorReturnType ValidatorDigits::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorDigits);

    const QString v = value(params);
    bool ok = false;
    int _length = d->extractInt(c, params, d->length, &ok);
    if (!ok) {
        result.errorMessage = validationDataError(c);
        return result;
    }

    if (!v.isEmpty()) {

        if (Q_LIKELY(ValidatorDigits::validate(v, _length))) {
            if ((_length > 0) && (v.length() != _length)) {
                result.errorMessage = validationError(c, _length);
                qCDebug(C_VALIDATOR, "ValidatorDigits: Validation failed for value \"%s\" in field %s at %s::%s: does not contain exactly %i digit(s).", qPrintable(v), qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), _length);
            } else {
                result.value.setValue<QString>(v);
            }
        } else {
            result.errorMessage = validationError(c, _length);
            qCDebug(C_VALIDATOR, "ValidatorDigits: Validation failed for value \"%s\" in field %s at %s::%s: does not only contain digits.", qPrintable(v), qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
        }

    } else {
        defaultValue(c, &result, "ValidatorDigits");
    }

    return result;
}

bool ValidatorDigits::validate(const QString &value, int length)
{
    bool valid = true;

    for (const QChar &ch : value) {
        const ushort &uc = ch.unicode();
        if (!((uc > 47) && (uc < 58))) {
            valid = false;
            break;
        }
    }

    if (valid && (length > 0) && (length != value.length())) {
        valid = false;
    }

    return valid;
}

QString ValidatorDigits::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;

    const QString _label = label(c);
    const int _length = errorData.toInt();

    if (_label.isEmpty()) {
        if (_length > 0) {
            error = c->translate("Cutelyst::ValidatorDigits", "Must contain exactly %n digit(s).", "", _length);
        } else {
            error = c->translate("Cutelyst::ValidatorDigits", "Must only contain digits.");
        }
    } else {
        if (_length > 0) {
            //: %1 will be replaced by the field label
            error = c->translate("Cutelyst::ValidatorDigits", "The “%1” field must contain exactly %n digit(s).", "", _length).arg(_label);
        } else {
            //: %1 will be replaced by the field label
            error = c->translate("Cutelyst::ValidatorDigits", "The “%1” field must only contain digits.").arg(_label);
        }
    }

    return error;
}
