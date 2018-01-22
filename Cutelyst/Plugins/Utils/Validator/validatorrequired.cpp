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

#include "validatorrequired_p.h"

using namespace Cutelyst;

ValidatorRequired::ValidatorRequired(const QString &field, const Cutelyst::ValidatorMessages &messages) :
    ValidatorRule(*new ValidatorRequiredPrivate(field, messages))
{
}

ValidatorRequired::~ValidatorRequired()
{
}

ValidatorReturnType ValidatorRequired::validate(Cutelyst::Context *c, const Cutelyst::ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    const QString v = value(params);
    if (Q_LIKELY(!v.isEmpty())) {
        result.value.setValue<QString>(v);
    } else {
        result.errorMessage = validationError(c);
    }

    return result;
}

QString ValidatorRequired::genericValidationError(Cutelyst::Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData);
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorRequired", "This is required.");
    } else {
        error = c->translate("Cutelyst::ValidatorRequired", "The “%1” field is required.").arg(_label);
    }
    return error;
}
