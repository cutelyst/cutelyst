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
    QString result;

    Q_D(const ValidatorIn);

    if (!d->values.isEmpty()) {
        if (!value().isEmpty() && !d->values.contains(value())) {
            result = validationError();
        }
    } else {
        result = validationDataError();
    }

    return result;
}

QString ValidatorIn::genericValidationError() const
{
    QString error;
    Q_D(const ValidatorIn);
    if (label().isEmpty()) {
        error = QStringLiteral("Has to be one of the following: %1").arg(d->values.join(QStringLiteral(", ")));
    } else {
        error = QStringLiteral("The value in the “%1” field has to be one of the following: %2").arg(label(), d->values.join(QStringLiteral(", ")));
    }
    return error;
}

void ValidatorIn::setValues(const QStringList &values)
{
    Q_D(ValidatorIn);
    d->values = values;
}
