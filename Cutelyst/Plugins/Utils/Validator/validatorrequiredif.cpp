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

#include "validatorrequiredif_p.h"

using namespace Cutelyst;

ValidatorRequiredIf::ValidatorRequiredIf(const QString &field, const QString &otherField, const QStringList &otherValues, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorRequiredIfPrivate(field, otherField, otherValues, label, customError))
{
}

ValidatorRequiredIf::ValidatorRequiredIf(ValidatorRequiredIfPrivate &dd) :
    ValidatorRule(dd)
{
}

ValidatorRequiredIf::~ValidatorRequiredIf()
{
}

QString ValidatorRequiredIf::validate() const
{
    QString result;

    Q_D(const ValidatorRequiredIf);

    if (d->otherField.isEmpty() || d->otherValues.empty()) {
        result = validationDataError();
    } else if (d->otherValues.contains(d->parameters.value(d->otherField)) && value().isEmpty()) {
        result = validationError();
    }

    return result;
}

QString ValidatorRequiredIf::genericValidationError() const
{
    return QStringLiteral("You must fill in the “%1” field.").arg(fieldLabel());
}

void ValidatorRequiredIf::setOtherField(const QString &otherField)
{
    Q_D(ValidatorRequiredIf);
    d->otherField = otherField;
}

void ValidatorRequiredIf::setOtherValues(const QStringList &otherValues)
{
    Q_D(ValidatorRequiredIf);
    d->otherValues = otherValues;
}
