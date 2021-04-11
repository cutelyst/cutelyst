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

#include "validatorfilled_p.h"

using namespace Cutelyst;

ValidatorFilled::ValidatorFilled(const QString &field, const Cutelyst::ValidatorMessages &messages, const QString &defValKey) :
    ValidatorRule(*new ValidatorFilledPrivate(field, messages, defValKey))
{
}

ValidatorFilled::~ValidatorFilled()
{
}

ValidatorReturnType ValidatorFilled::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    if (params.contains(field())) {
        const QString v = value(params);
        if (!v.isEmpty()) {
            result.value.setValue(v);
        } else {
            result.errorMessage = validationError(c);
        }
    } else {
        defaultValue(c, &result, "ValidatorAfter");
    }

    return result;
}

QString ValidatorFilled::genericValidationError(Context *c, const QVariant &errorData) const
{
    Q_UNUSED(errorData)
    QString error;
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorFilled", "Must be filled.");
    } else {
        //: %1 will be replaced by the field label
        error = c->translate("Cutelyst::ValidatorFilled", "You must fill in the “%1” field.");
    }
    return error;
}
