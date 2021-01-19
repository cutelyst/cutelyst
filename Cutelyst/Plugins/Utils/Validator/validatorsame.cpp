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

#include "validatorsame_p.h"

using namespace Cutelyst;

ValidatorSame::ValidatorSame(const QString &field, const QString &otherField, const char *otherLabel, const Cutelyst::ValidatorMessages &messages, const QString &defValKey) :
    ValidatorRule(*new ValidatorSamePrivate(field, otherField, otherLabel, messages, defValKey))
{
}

ValidatorSame::~ValidatorSame()
{
}

ValidatorReturnType ValidatorSame::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorSame);

    const QString v = value(params);

    if (!v.isEmpty()) {
        const QString ov = trimBefore() ? params.value(d->otherField).trimmed() : params.value(d->otherField);
        if (v != ov) {
            result.errorMessage = validationError(c);
            qCDebug(C_VALIDATOR, "ValidatorSame: Validation failed for field %s at %s::%s: value is not the same as in the field %s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), qPrintable(d->otherField));
        } else {
            result.value.setValue(v);
        }
    } else {
        defaultValue(c, &result, "ValidatorSame");
    }

    return result;
}

QString ValidatorSame::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;

    Q_D(const ValidatorSame);
    Q_UNUSED(errorData)
    const QString _label = label(c);
    QString _olabel;
    if (d->otherLabel) {
        _olabel = d->translationContext.size() ? c->translate(d->translationContext.data(), d->otherLabel) : QString::fromUtf8(d->otherLabel);
    } else {
        _olabel = d->otherField;
    }

    if (_label.isEmpty()) {
        //: %1 will be replaced by the label of the other field
        error = c->translate("Cutelyst::ValidatorSame", "Must be the same as in the “%1” field.").arg(_olabel);
    } else {
        //: %1 will be replaced by the field label, %2 will be replaced by the label of the other field
        error = c->translate("Cutelyst::ValidatorSame", "The “%1” field must have the same value as the “%2” field.").arg(_label, _olabel);
    }

    return error;
}
