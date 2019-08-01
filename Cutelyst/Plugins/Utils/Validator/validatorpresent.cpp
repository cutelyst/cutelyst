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

#include "validatorpresent_p.h"

using namespace Cutelyst;

ValidatorPresent::ValidatorPresent(const QString &field, const Cutelyst::ValidatorMessages &messages) :
    ValidatorRule(*new ValidatorPresentPrivate(field, messages))
{
}

ValidatorPresent::~ValidatorPresent()
{
}

ValidatorReturnType ValidatorPresent::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    if (!params.contains(field())) {
        result.errorMessage = validationError(c);
        qCDebug(C_VALIDATOR, "ValidatorPresent: Validation failed for field %s at %s::%s: field was not found in the input data", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
    } else {
        result.value.setValue<QString>(value(params));
    }

    return result;
}

QString ValidatorPresent::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorPresent", "Has to be present in input data.");
    } else {
        //: %1 will be replaced by the field label
        error =  c->translate("Cutelyst::ValidatorPresent", "The “%1” field was not found in the input data.").arg(_label);
    }
    return error;
}
