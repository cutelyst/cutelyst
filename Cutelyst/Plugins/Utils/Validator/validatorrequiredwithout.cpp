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

#include "validatorrequiredwithout_p.h"

using namespace Cutelyst;

ValidatorRequiredWithout::ValidatorRequiredWithout(const QString &field, const QStringList &otherFields, const Cutelyst::ValidatorMessages &messages) :
    ValidatorRule(*new ValidatorRequiredWithoutPrivate(field, otherFields, messages))
{
}

ValidatorRequiredWithout::~ValidatorRequiredWithout()
{
}

ValidatorReturnType ValidatorRequiredWithout::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorRequiredWithout);

    if (d->otherFields.isEmpty()) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR, "ValidatorRequiredWithout: invalid validation data for field %s at %s::%s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
    } else {

        bool otherMissing = false;

        const QStringList ofc = d->otherFields;

        for (const QString &other : ofc) {
            if (!params.contains(other)) {
                otherMissing = true;
                break;
            }
        }

        const QString v = value(params);

        if (otherMissing) {
            if (!v.isEmpty()) {
                result.value.setValue<QString>(v);
            } else {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR, "ValidatorRequiredWithout: Validation failed for field %s at %s::%s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            }
        } else {
            if (!v.isEmpty()) {
                result.value.setValue<QString>(v);
            }
        }
    }

    return result;
}

QString ValidatorRequiredWithout::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorRequiredWithout", "This is required.");
    } else {        
        error = c->translate("Cutelyst::ValidatorRequiredWithout", "You must fill in the “%1” field.").arg(_label);
    }
    return error;
}

