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

#include "validatornumeric_p.h"
#include <QRegularExpression>

using namespace Cutelyst;

ValidatorNumeric::ValidatorNumeric(const QString &field, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorNumericPrivate(field, label, customError))
{
}

ValidatorNumeric::ValidatorNumeric(ValidatorNumericPrivate &dd) :
    ValidatorRule(dd)
{
}

ValidatorNumeric::~ValidatorNumeric()
{
}

QString ValidatorNumeric::validate() const
{
    if (value().isEmpty()) {
        return QString();
    }

    if (value().contains(QRegularExpression(QStringLiteral("^-?\\d+(\\.|,)?\\d*(e|E)?\\d+$")))) {
        return QString();
    }

    return validationError();
}

QString ValidatorNumeric::genericValidationError() const
{
    return QStringLiteral("You have to enter a numeric value into the “%1” field, like 1, -2.5 or 3.454e3").arg(fieldLabel());
}
