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

#include "validatorrequiredunless_p.h"

using namespace Cutelyst;

ValidatorRequiredUnless::ValidatorRequiredUnless(const QString &field, const QString &otherField, const QStringList &otherValues, const QString &label, const QString &customError, QObject *parent) :
    ValidatorRule(*new ValidatorRequiredUnlessPrivate(field, otherField, otherValues, label, customError), parent)
{
}

ValidatorRequiredUnless::ValidatorRequiredUnless(ValidatorRequiredUnlessPrivate &dd, QObject *parent) :
    ValidatorRule(dd, parent)
{
}

ValidatorRequiredUnless::~ValidatorRequiredUnless()
{
}

bool ValidatorRequiredUnless::validate()
{
    Q_D(ValidatorRequiredUnless);

    if (d->otherField.isEmpty()) {
        setValidationDataError(true);
        return false;
    }

    if (d->otherValues.isEmpty()) {
        setValidationDataError(true);
        return false;
    }

    if (!d->otherValues.contains(d->parameters.value(d->otherField))) {

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

QString ValidatorRequiredUnless::genericErrorMessage() const
{
    return tr("You must fill in the “%1” field.").arg(genericFieldName());
}

void ValidatorRequiredUnless::setOtherField(const QString &otherField)
{
    Q_D(ValidatorRequiredUnless);
    d->otherField = otherField;
}

void ValidatorRequiredUnless::setOtherValues(const QStringList &otherValues)
{
    Q_D(ValidatorRequiredUnless);
    d->otherValues = otherValues;
}
