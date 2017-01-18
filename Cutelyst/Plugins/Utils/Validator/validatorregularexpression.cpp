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

ValidatorRegularExpression::ValidatorRegularExpression(const QString &field, const QRegularExpression &regex, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorRegularExpressionPrivate(field, regex, label, customError))
{
}

ValidatorRegularExpression::ValidatorRegularExpression(ValidatorRegularExpressionPrivate &dd) :
    ValidatorRule(dd)
{
}

ValidatorRegularExpression::~ValidatorRegularExpression()
{
}

QString ValidatorRegularExpression::validate() const
{
    QString result;

    Q_D(const ValidatorRegularExpression);

    if (!value().isEmpty() && !value().contains(d->regex)) {
        result = validationError();
    }

    return result;
}

QString ValidatorRegularExpression::genericValidationError() const
{
    QString error;
    if (label().isEmpty()) {
        error = QStringLiteral("Does not match desired format.");
    } else {
        error = QStringLiteral("The “%1” field does not match the desired format.").arg(label());
    }
    return error;
}

void ValidatorRegularExpression::setRegex(const QRegularExpression &regex)
{
    Q_D(ValidatorRegularExpression);
    d->regex = regex;
}
