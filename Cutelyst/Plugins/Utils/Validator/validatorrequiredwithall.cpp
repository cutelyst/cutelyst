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

#include "validatorrequiredwithall_p.h"

using namespace Cutelyst;

ValidatorRequiredWithAll::ValidatorRequiredWithAll(const QString &field, const QStringList &otherFields, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorRequiredWithAllPrivate(field, otherFields, label, customError))
{
}

ValidatorRequiredWithAll::ValidatorRequiredWithAll(ValidatorRequiredWithAllPrivate &dd) :
    ValidatorRule(dd)
{
}

ValidatorRequiredWithAll::~ValidatorRequiredWithAll()
{
}

bool ValidatorRequiredWithAll::validate()
{
    Q_D(ValidatorRequiredWithAll);

    if (d->otherFields.isEmpty()) {
        setValidationDataError(true);
        return false;
    }

    bool containsAll = true;

    const QStringList ofc = d->otherFields;

    for (const QString &other : ofc) {
        if (!parameters().contains(other)) {
            containsAll = false;
            break;
        }
    }

    if (containsAll) {

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

QString ValidatorRequiredWithAll::genericErrorMessage() const
{
    return QStringLiteral("You must fill in the “%1” field.").arg(genericFieldName());
}

void ValidatorRequiredWithAll::setOtherFields(const QStringList &otherFields)
{
    Q_D(ValidatorRequiredWithAll);
    d->otherFields = otherFields;
}
