﻿/*
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

#include "validatorinteger_p.h"
#include <QRegularExpression>

using namespace Cutelyst;

ValidatorInteger::ValidatorInteger(const QString &field, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorIntegerPrivate(field, label, customError))
{
}


ValidatorInteger::ValidatorInteger(ValidatorIntegerPrivate &dd) :
    ValidatorRule(dd)
{
}


ValidatorInteger::~ValidatorInteger()
{
}

QString ValidatorInteger::validate() const
{
    QString result;

    const QString v = value();

    if (!v.isEmpty() && !v.contains(QRegularExpression(QStringLiteral("^-?\\d+$")))) {
        result = validationError();
    }

    return result;
}

QString ValidatorInteger::genericValidationError() const
{
    QString error;
    if (label().isEmpty()) {
        error = QStringLiteral("Has to be an integer (1,2,-3 etc).");
    } else {
        error = QStringLiteral("You have to enter an integer (1,2,-3 etc.) into the “%1” field.").arg(label());
    }
    return error;
}
