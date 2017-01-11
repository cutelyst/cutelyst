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

#include "validatorin_p.h"

using namespace Cutelyst;

ValidatorIn::ValidatorIn(const QString &field, const QStringList &values, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorInPrivate(field, values, label, customError))
{
}

ValidatorIn::ValidatorIn(ValidatorInPrivate &dd) :
    ValidatorRule(dd)
{
}

ValidatorIn::~ValidatorIn()
{
}

QString ValidatorIn::validate() const
{
    Q_D(const ValidatorIn);

    if (d->values.isEmpty()) {
        return validationDataError();
    }

    if (value().isEmpty()) {
        return QString();
    }

    if (d->values.contains(value())) {
        return QString();
    }

    return validationError();
}

QString ValidatorIn::genericValidationError() const
{
    Q_D(const ValidatorIn);
    return QStringLiteral("The value in the “%1“ field has to be one of the following: %2").arg(fieldLabel(), d->values.join(QStringLiteral(", ")));
}

void ValidatorIn::setValues(const QStringList &values)
{
    Q_D(ValidatorIn);
    d->values = values;
}
