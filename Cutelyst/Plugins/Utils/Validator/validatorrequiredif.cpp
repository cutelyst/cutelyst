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

ValidatorRequiredIf::ValidatorRequiredIf(const QString &field, const QString &otherField, const QStringList &otherValues, const QString &label, const QString &customError, QObject *parent) :
    ValidatorRule(*new ValidatorRequiredIfPrivate(field, otherField, otherValues, label, customError), parent)
{
}

ValidatorRequiredIf::ValidatorRequiredIf(ValidatorRequiredIfPrivate &dd, QObject *parent) :
    ValidatorRule(dd, parent)
{
}

ValidatorRequiredIf::~ValidatorRequiredIf()
{
}

bool ValidatorRequiredIf::validate()
{
    Q_D(ValidatorRequiredIf);

    if (d->otherField.isEmpty()) {
        setValidationDataError(true);
        return false;
    }

    if (d->otherValues.isEmpty()) {
        setValidationDataError(true);
        return false;
    }

    if (d->otherValues.contains(d->parameters.value(d->otherField))) {

        if (!value().isEmpty()) {
            setValid(true);
            return true;
        }

    } else {
        setValid(true);
        return true;
    }

    return false;
}

QString ValidatorRequiredIf::genericErrorMessage() const
{
    return tr("You must fill in the “%1” field.").arg(genericFieldName());
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
