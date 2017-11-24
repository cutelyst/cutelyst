/*
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

QString ValidatorBoolean::validate() const
{
    QString result;

    if (!value().isEmpty()) {
        static const QStringList l({QStringLiteral("1"), QStringLiteral("0"), QStringLiteral("true"), QStringLiteral("false"), QStringLiteral("on"), QStringLiteral("off")});
        if (!l.contains(value(), Qt::CaseInsensitive)) {
            result = validationError();
        }
    }

    return result;
}

QString ValidatorBoolean::genericValidationError() const
{
    QString error;
    if (label().isEmpty()) {
        error = QStringLiteral("Can not be interpreted as boolean.");
    } else {
        error = QStringLiteral("The data in the “%1” field can not be interpreted as a boolean.").arg(label());
    }
    return error;
}

