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

ValidatorRequiredUnless::ValidatorRequiredUnless(const QString &field, const QString &otherField, const QStringList &otherValues, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorRequiredUnlessPrivate(field, otherField, otherValues, label, customError))
{
}

ValidatorRequiredUnless::ValidatorRequiredUnless(ValidatorRequiredUnlessPrivate &dd) :
    ValidatorRule(dd)
{
}

ValidatorRequiredUnless::~ValidatorRequiredUnless()
{
}

QString ValidatorRequiredUnless::validate() const
{
    QString result;

    Q_D(const ValidatorRequiredUnless);

    if (d->otherField.isEmpty() || d->otherValues.empty()) {
        result = validationDataError();
    } else if (!d->otherValues.contains(d->parameters.value(d->otherField)) && value().isEmpty()) {
        result = validationError();
    }

    return result;
}

QString ValidatorRequiredUnless::genericValidationError() const
{
    QString error;
    if (label().isEmpty()) {
        error = QStringLiteral("This is required.");
    } else {
        error = QStringLiteral("You must fill in the “%1” field.").arg(label());
    }
    return error;
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
