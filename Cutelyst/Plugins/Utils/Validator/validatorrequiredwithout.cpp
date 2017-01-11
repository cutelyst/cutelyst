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

#include "validatorrequiredwithout_p.h"

using namespace Cutelyst;

ValidatorRequiredWithout::ValidatorRequiredWithout(const QString &field, const QStringList &otherFields, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorRequiredWithoutPrivate(field, otherFields, label, customError))
{
}

ValidatorRequiredWithout::ValidatorRequiredWithout(ValidatorRequiredWithoutPrivate &dd) :
    ValidatorRule(dd)
{
}

ValidatorRequiredWithout::~ValidatorRequiredWithout()
{
}

QString ValidatorRequiredWithout::validate() const
{
    Q_D(const ValidatorRequiredWithout);

    if (d->otherFields.isEmpty()) {
        return validationDataError();
    }

    bool otherMissing = false;

    const QStringList ofc = d->otherFields;

    for (const QString &other : ofc) {
        if (!d->parameters.contains(other)) {
            otherMissing = true;
            break;
        }
    }

    if (otherMissing) {
        if (!value().isEmpty()) {
            return QString();
        } else {
            return validationError();
        }
    } else {
        return QString();
    }

    return validationError();
}

QString ValidatorRequiredWithout::genericValidationError() const
{
    return QStringLiteral("You must fill in the “%1” field.").arg(fieldLabel());
}

void ValidatorRequiredWithout::setOtherFields(const QStringList &otherFields)
{
    Q_D(ValidatorRequiredWithout);
    d->otherFields = otherFields;
}
