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

#include "validatornotin_p.h"

using namespace Cutelyst;

ValidatorNotIn::ValidatorNotIn(const QString &field, const QStringList &values, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorNotInPrivate(field, values, label, customError))
{
}

ValidatorNotIn::ValidatorNotIn(ValidatorNotInPrivate &dd) :
    ValidatorRule(dd)
{
}

ValidatorNotIn::~ValidatorNotIn()
{
}

QString ValidatorNotIn::validate() const
{
    QString result;

    Q_D(const ValidatorNotIn);

    if (d->values.empty()) {
        result = validationDataError();
    } else {
        const QString v = value();
        if (!v.isEmpty() && d->values.contains(v)) {
            result = validationError();
        }
    }

    return result;
}

QString ValidatorNotIn::genericValidationError() const
{
    QString error;
    if (label().isEmpty()) {
        error = QStringLiteral("Value is not allowed.");
    } else {
        error = QStringLiteral("The value in the “%1” field is not allowed.").arg(label());
    }
    return error;
}

void ValidatorNotIn::setValues(const QStringList &values)
{
    Q_D(ValidatorNotIn);
    d->values = values;
}
