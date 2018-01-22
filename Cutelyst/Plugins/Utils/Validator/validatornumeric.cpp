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

#include "validatornumeric_p.h"

using namespace Cutelyst;

ValidatorNumeric::ValidatorNumeric(const QString &field, const Cutelyst::ValidatorMessages &messages, const QString &defValKey) :
    ValidatorRule(*new ValidatorNumericPrivate(field, messages, defValKey))
{
}

ValidatorNumeric::~ValidatorNumeric()
{
}

ValidatorReturnType ValidatorNumeric::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    const QString v = value(params);

    if (!v.isEmpty()) {
        bool ok = false;
        const double _v = v.toDouble(&ok);
        if (Q_LIKELY(ok)) {
            result.value.setValue<double>(_v);
        } else {
            qCDebug(C_VALIDATOR, "ValidatorNumeric: Validation failed for field %s at %s::%s: can not convert input value into a numeric value.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            result.errorMessage = validationError(c);
        }
    } else {
        defaultValue(c, &result, "ValidatorNumeric");
    }

    return result;
}

QString ValidatorNumeric::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorNumeric", "Must be numeric, like 1, -2.5 or 3.454e3.");
    } else {
        error = c->translate("Cutelyst::ValidatorNumeric", "You have to enter a numeric value into the “%1” field, like 1, -2.5 or 3.454e3").arg(_label);
    }
    return error;
}
