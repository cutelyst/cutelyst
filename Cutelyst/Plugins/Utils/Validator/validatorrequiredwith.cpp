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

#include "validatorrequiredwith_p.h"

using namespace Cutelyst;

ValidatorRequiredWith::ValidatorRequiredWith(const QString &field, const QStringList &otherFields, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorRequiredWithPrivate(field, otherFields, label, customError))
{
}

ValidatorRequiredWith::ValidatorRequiredWith(ValidatorRequiredWithPrivate &dd) :
    ValidatorRule(dd)
{
}

ValidatorRequiredWith::~ValidatorRequiredWith()
{
}

bool ValidatorRequiredWith::validate()
{
    Q_D(ValidatorRequiredWith);

    if (d->otherFields.isEmpty()) {
        setValidationDataError(true);
        return false;
    }

    bool containsOther = false;

    const QStringList ofc = d->otherFields;

    for (const QString &other : ofc)  {
        if (parameters().contains(other)) {
            containsOther = true;
            break;
        }
    }

    if (containsOther) {
        if (!value().isEmpty()) {
            setValid(true);
            return true;
        } else {
            return false;
        }
    } else {
        setValid(true);
        return true;
    }

    return false;
}

QString ValidatorRequiredWith::genericErrorMessage() const
{
    return QStringLiteral("You must fill in the “%1” field.").arg(genericFieldName());
}

void ValidatorRequiredWith::setOtherFields(const QStringList &otherFields)
{
    Q_D(ValidatorRequiredWith);
    d->otherFields = otherFields;
}
