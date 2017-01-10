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

#include "validatorboolean_p.h"
#include <QStringList>

using namespace Cutelyst;

ValidatorBoolean::ValidatorBoolean(const QString &field, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorBooleanPrivate(field, label, customError))
{
}

ValidatorBoolean::ValidatorBoolean(ValidatorBooleanPrivate &dd) :
    ValidatorRule(dd)
{
}

ValidatorBoolean::~ValidatorBoolean()
{
}

bool ValidatorBoolean::validate()
{
    if (value().isEmpty()) {
        setError(ValidatorRule::NoError);
        return true;
    }

    QStringList l({QStringLiteral("1"), QStringLiteral("0"), QStringLiteral("true"), QStringLiteral("false"), QStringLiteral("on"), QStringLiteral("off")});
    if (l.contains(value(), Qt::CaseInsensitive)) {
        setError(ValidatorRule::NoError);
        return true;
    } else {
        return false;
    }
}

QString ValidatorBoolean::genericErrorMessage() const
{
    return QStringLiteral("The data in the “%1” field can not be interpreted as a boolean.").arg(genericFieldName());
}

