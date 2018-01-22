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

#include "validatorrequiredwith_p.h"

using namespace Cutelyst;

ValidatorRequiredWith::ValidatorRequiredWith(const QString &field, const QStringList &otherFields, const Cutelyst::ValidatorMessages &messages) :
    ValidatorRule(*new ValidatorRequiredWithPrivate(field, otherFields, messages))
{
}

ValidatorRequiredWith::~ValidatorRequiredWith()
{
}

ValidatorReturnType ValidatorRequiredWith::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorRequiredWith);

    if (d->otherFields.empty()) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR, "ValidatorRequiredWith: invalid validation data for field %s at %s::%s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
    } else {
        bool containsOther = false;
        const QString v = value(params);

        const QStringList ofc = d->otherFields;

        for (const QString &other : ofc)  {
            if (params.contains(other)) {
                containsOther = true;
                break;
            }
        }

        if (containsOther) {
            if (!v.isEmpty()) {
                result.value.setValue<QString>(v);
            } else {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR, "ValidatorRequiredWith: Validation failed for field %s at %s::%s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            }
        } else {
            if (!v.isEmpty()) {
                result.value.setValue<QString>(v);
            }
        }
    }

    return result;
}

QString ValidatorRequiredWith::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData);
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorRequiredWith", "This is required.");
    } else {
        error = c->translate("Cutelyst::ValidatorRequiredWith", "You must fill in the “%1” field.").arg(_label);
    }
    return error;
}
