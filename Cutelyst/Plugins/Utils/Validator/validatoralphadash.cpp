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

#include "validatoralphadash_p.h"
#include <QRegularExpression>

using namespace Cutelyst;

ValidatorAlphaDash::ValidatorAlphaDash(const QString &field, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorAlphaDashPrivate(field, label, customError))
{

}

ValidatorAlphaDash::ValidatorAlphaDash(ValidatorAlphaDashPrivate &dd) :
    ValidatorRule(dd)
{

}

ValidatorAlphaDash::~ValidatorAlphaDash()
{

}

QString ValidatorAlphaDash::validate() const
{
    QString result;

    if (!value().isEmpty() && !value().contains(QRegularExpression(QStringLiteral("^[\\pL\\pM\\pN_-]+$")))) {
        result = validationError();
    }

    return result;
}

QString ValidatorAlphaDash::genericValidationError() const
{
    return QStringLiteral("The %1 field can only contain alpha-numeric characters, as well as dashes and underscores, but nothing else.").arg(fieldLabel());
}
