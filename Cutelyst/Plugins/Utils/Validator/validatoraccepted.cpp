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

#include "validatoraccepted_p.h"

using namespace Cutelyst;

ValidatorAccepted::ValidatorAccepted(const QString &field, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorRulePrivate(field, label, customError))
{

}

ValidatorAccepted::ValidatorAccepted(ValidatorAcceptedPrivate &dd) :
    ValidatorRule(dd)
{

}

ValidatorAccepted::~ValidatorAccepted()
{

}

QString ValidatorAccepted::validate() const
{
    QString result;

    static const QStringList l({QStringLiteral("yes"), QStringLiteral("on"), QStringLiteral("1"), QStringLiteral("true")});
    if (!l.contains(value(), Qt::CaseInsensitive)) {
        result = validationError();
    }

    return result;
}

QString ValidatorAccepted::genericValidationError() const
{
    return QStringLiteral("The “%1” has to be accepted.").arg(fieldLabel());
}
