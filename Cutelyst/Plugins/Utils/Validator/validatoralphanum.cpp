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

#include "validatoralphanum_p.h"
#include <QRegularExpression>

using namespace Cutelyst;

ValidatorAlphaNum::ValidatorAlphaNum(const QString &field, bool asciiOnly, const ValidatorMessages &messages, const QString &defValKey) :
    ValidatorRule(*new ValidatorAlphaNumPrivate(field, asciiOnly, messages, defValKey))
{

}

ValidatorAlphaNum::~ValidatorAlphaNum()
{

}

ValidatorReturnType ValidatorAlphaNum::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorAlphaNum);

    const QString v = value(params);
    if (!v.isEmpty()) {
        if (Q_LIKELY(ValidatorAlphaNum::validate(v, d->asciiOnly))) {
            result.value.setValue(v);
        } else {
            qCDebug(C_VALIDATOR, "ValidatorAlphaNum: Validation failed for field %s at %s::%s: %s contains characters that are not allowed.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), qPrintable(v));
            result.errorMessage = validationError(c);
        }

    } else {
        defaultValue(c, &result, "ValidatorAlphaNum");
    }

    return result;
}

bool ValidatorAlphaNum::validate(const QString &value, bool asciiOnly)
{
    bool valid = true;
    if (asciiOnly) {
        for (const QChar &ch : value) {
            const ushort &uc = ch.unicode();
            if (!(((uc > 64) && (uc < 91)) || ((uc > 96) && (uc < 123)) || ((uc > 47) && (uc < 58)))) {
                valid = false;
                break;
            }
        }
    } else {
        valid = value.contains(QRegularExpression(QStringLiteral("^[\\pL\\pM\\pN]+$")));
    }
    return valid;
}

QString ValidatorAlphaNum::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    Q_D(const ValidatorAlphaNum);
    const QString _label = label(c);
    if (_label.isEmpty()) {
        if (d->asciiOnly) {
            error = c->translate("Cutelyst::ValidatorAlphaNum", "Must only contain alpha-numeric latin characters.");
        } else {
            error = c->translate("Cutelyst::ValidatorAlphaNum", "Must only contain alpha-numeric characters.");
        }
    } else {
        if (d->asciiOnly) {
            //: %1 will be replaced by the field label
            error = c->translate("Cutelyst::ValidatorAlphaNum", "The text in the “%1” field must only contain alpha-numeric latin characters.").arg(_label);
        } else {
            //: %1 will be replaced by the field label
            error = c->translate("Cutelyst::ValidatorAlphaNum", "The text in the “%1” field must only contain alpha-numeric characters.").arg(_label);
        }
    }
    return error;
}
