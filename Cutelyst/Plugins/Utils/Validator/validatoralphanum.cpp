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

#include "validatoralphanum_p.h"
#include <QRegularExpression>

using namespace Cutelyst;

ValidatorAlphaNum::ValidatorAlphaNum(const QString &field, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorAlphaNumPrivate(field, label, customError))
{

}

ValidatorAlphaNum::ValidatorAlphaNum(ValidatorAlphaNumPrivate &dd) :
    ValidatorRule(dd)
{

}

ValidatorAlphaNum::~ValidatorAlphaNum()
{

}

QString ValidatorAlphaNum::validate() const
{
    if (value().isEmpty()) {
        return QString();
    }

    if (value().contains(QRegularExpression(QStringLiteral("^[\\pL\\pM\\pN]+$")))) {
        return QString();
    } else {
        return validationError();
    }
}

QString ValidatorAlphaNum::genericValidationError() const
{
    return QStringLiteral("The text in the “%1” field must be entirely alpha-numeric characters.").arg(fieldLabel());
}
