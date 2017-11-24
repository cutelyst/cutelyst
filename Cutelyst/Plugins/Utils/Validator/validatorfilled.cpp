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

#include "validatorfilled_p.h"

using namespace Cutelyst;

ValidatorFilled::ValidatorFilled(const QString &field, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorFilledPrivate(field, label, customError))
{
}

ValidatorFilled::ValidatorFilled(ValidatorFilledPrivate &dd) :
    ValidatorRule(dd)
{
}

ValidatorFilled::~ValidatorFilled()
{
}

QString ValidatorFilled::validate() const
{
    QString result;

    if (parameters().contains(field()) && value().isEmpty()) {
        result = validationError();
    }

    return result;
}

QString ValidatorFilled::genericValidationError() const
{
    QString error;
    if (label().isEmpty()) {
        error = QStringLiteral("Must be filled.");
    } else {
        error = QStringLiteral("You must fill in the “%1” field.").arg(label());
    }
    return error;
}
