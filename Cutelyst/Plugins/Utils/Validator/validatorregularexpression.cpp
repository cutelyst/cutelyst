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

#include "validatorregularexpression_p.h"

using namespace Cutelyst;

ValidatorRegularExpression::ValidatorRegularExpression(const QString &field, const QRegularExpression &regex, const QString &label, const QString &customError, QObject *parent) :
    ValidatorRule(*new ValidatorRegularExpressionPrivate(field, regex, label, customError), parent)
{

}


ValidatorRegularExpression::ValidatorRegularExpression(ValidatorRegularExpressionPrivate &dd, QObject *parent) :
    ValidatorRule(dd, parent)
{

}


ValidatorRegularExpression::~ValidatorRegularExpression()
{

}



bool ValidatorRegularExpression::validate()
{
    if (value().isEmpty()) {
        setValid(true);
        return true;
    }

    Q_D(ValidatorRegularExpression);

    if (value().contains(d->regex)) {
        setValid(true);
        return true;
    }

    return false;
}



QString ValidatorRegularExpression::genericErrorMessage() const
{
    Q_D(const ValidatorRegularExpression);
    return tr("The “%1” field does not match the desired format.").arg(genericFieldName());
}


void ValidatorRegularExpression::setRegex(const QRegularExpression &regex)
{
    Q_D(ValidatorRegularExpression);
    d->regex = regex;
}

