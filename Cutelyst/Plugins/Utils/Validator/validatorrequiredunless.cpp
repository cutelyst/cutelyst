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

#include "validatorrequiredunless_p.h"

using namespace Cutelyst;

ValidatorRequiredUnless::ValidatorRequiredUnless(const QString &field, const QString &otherField, const QStringList &otherValues, const Cutelyst::ValidatorMessages &messages) :
    ValidatorRule(*new ValidatorRequiredUnlessPrivate(field, otherField, otherValues, messages))
{
}

ValidatorRequiredUnless::~ValidatorRequiredUnless()
{
}

ValidatorReturnType ValidatorRequiredUnless::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorRequiredUnless);

    if (d->otherField.isEmpty() || d->otherValues.empty()) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR, "ValidatorRequiredUnless: invalid validation data for field %s at %s::%s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
    } else {
        const QString v = value(params);
        const QString ov = trimBefore() ? params.value(d->otherField).trimmed() : params.value(d->otherField);
        if (!d->otherValues.contains(ov)) {
            if (!v.isEmpty()) {
                result.value.setValue<QString>(v);
            } else {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR, "ValidatorRequiredUnless: Validation failed for field %s at %s::%s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            }
        } else {
            if (!v.isEmpty()) {
                result.value.setValue<QString>(v);
            }
        }
    }

    return result;
}

QString ValidatorRequiredUnless::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorRequiredUnless", "This is required.");
    } else {
        //: %1 will be replaced by the field label
        error = c->translate("Cutelyst::ValidatorRequiredUnless", "The “%1” field is required.").arg(_label);
    }
    return error;
}
