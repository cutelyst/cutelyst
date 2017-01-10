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

bool ValidatorNotIn::validate()
{
    Q_D(ValidatorNotIn);

    if (d->values.isEmpty()) {
        setError(ValidatorRule::ValidationDataError);
        return false;
    }

    QString v = value();

    if (v.isEmpty()) {
        setError(ValidatorRule::NoError);
        return true;
    }

    if (!d->values.contains(v)) {
        setError(ValidatorRule::NoError);
        return true;
    }

    return false;
}

QString ValidatorNotIn::genericErrorMessage() const
{
    return QStringLiteral("The value in the “%1“ field is not allowed.").arg(genericFieldName());
}

void ValidatorNotIn::setValues(const QStringList &values)
{
    Q_D(ValidatorNotIn);
    d->values = values;
}
