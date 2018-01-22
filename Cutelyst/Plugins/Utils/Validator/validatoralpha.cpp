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

#include "validatoralpha_p.h"
#include <QRegularExpression>

using namespace Cutelyst;

ValidatorAlpha::ValidatorAlpha(const QString &field, bool asciiOnly, const Cutelyst::ValidatorMessages &messages, const QString &defValKey) :
    ValidatorRule(*new ValidatorAlphaPrivate(field, asciiOnly, messages, defValKey))
{

}

ValidatorAlpha::~ValidatorAlpha()
{

}

ValidatorReturnType ValidatorAlpha::validate(Cutelyst::Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorAlpha);

    const QString v = value(params);
    if (!v.isEmpty()) {
        if (Q_LIKELY(ValidatorAlpha::validate(v, d->asciiOnly))) {
            result.value.setValue<QString>(v);
            qCDebug(C_VALIDATOR, "ValidatorAlhpa: Validation failed for field %s at %s::%s: %s contains characters that are not allowed.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), qPrintable(v));
        } else {
            result.errorMessage = validationError(c);
        }
    } else {
        defaultValue(c, &result, "ValidatorAlpha");
    }

    return result;
}

bool ValidatorAlpha::validate(const QString &value, bool asciiOnly)
{
    bool valid = true;

    if (asciiOnly) {
        for (const QChar &ch : value) {
            const ushort &uc = ch.unicode();
            if (!(((uc > 64) && (uc < 91)) || ((uc > 96) && (uc < 123)))) {
                valid = false;
                break;
            }
        }
    } else {
        valid = value.contains(QRegularExpression(QStringLiteral("^[\\pL\\pM]+$")));
    }

    return valid;
}

QString ValidatorAlpha::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorAlhpa", "Must be entirely alphabetic characters.");
    } else {
        error = c->translate("Cutelyst::ValidatorAlhpa", "The text in the “%1” field must be entirely alphabetic characters.").arg(_label);
    }
    return error;
}
