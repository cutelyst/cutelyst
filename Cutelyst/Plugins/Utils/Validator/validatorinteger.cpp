/*
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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

bool ValidatorInteger::validate()
{
    QString v = value();

    if (v.isEmpty()) {
        setValid(true);
        return true;
    }

    if (value().contains(QRegularExpression(QStringLiteral("^-?\\d+$")))) {
        setValid(true);
        return true;
    }

    return false;
}

QString ValidatorInteger::genericErrorMessage() const
{
    return QStringLiteral("You have to enter an integer (1,2,-3 etc.) into the “%1” field.").arg(genericFieldName());
}
