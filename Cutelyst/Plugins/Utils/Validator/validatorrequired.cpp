/*
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
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

ValidatorRequired::ValidatorRequired(const QString &field, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorRequiredPrivate(field, label, customError))
{
}

ValidatorRequired::ValidatorRequired(ValidatorRequiredPrivate &dd) :
    ValidatorRule(dd)
{
}

ValidatorRequired::~ValidatorRequired()
{
}

QString ValidatorRequired::validate() const
{
    QString result;

    if (value().isEmpty()) {
        result = validationError();
    }

    return result;
}

QString ValidatorRequired::genericValidationError() const
{
    QString error;
    if (label().isEmpty()) {
        error = QStringLiteral("This is required.");
    } else {
        error = QStringLiteral("You must fill in the “%1” field.").arg(label());
    }
    return error;
}
