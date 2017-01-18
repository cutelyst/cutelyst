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
    QString result;

    if (!value().isEmpty() && !value().contains(QRegularExpression(QStringLiteral("^-?\\d+(\\.|,)?\\d*(e|E)?\\d+$")))) {
        result = validationError();
    }

    return result;
}

QString ValidatorNumeric::genericValidationError() const
{
    QString error;
    if (label().isEmpty()) {
        error = QStringLiteral("Must be numeric, like 1, -2.5 or 3.454e3.");
    } else {
        error = QStringLiteral("You have to enter a numeric value into the “%1” field, like 1, -2.5 or 3.454e3").arg(label());
    }
    return error;
}
