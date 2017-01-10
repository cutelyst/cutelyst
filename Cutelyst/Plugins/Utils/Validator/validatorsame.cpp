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

#include "validatorsame_p.h"

using namespace Cutelyst;

ValidatorSame::ValidatorSame(const QString &field, const QString &otherField, const QString &label, const QString &otherLabel, const QString &customError, QObject *parent) :
    ValidatorRule(*new ValidatorSamePrivate(field, otherField, label, otherLabel, customError), parent)
{
}

ValidatorSame::ValidatorSame(ValidatorSamePrivate &dd, QObject *parent) :
    ValidatorRule(dd, parent)
{
}

ValidatorSame::~ValidatorSame()
{
}

bool ValidatorSame::validate()
{
    Q_D(ValidatorSame);

    QString v = value();

    if (v.isEmpty()) {
        setValid(true);
        return true;
    }

    if (v == d->parameters.value(d->otherField)) {
        setValid(true);
        return true;
    }

    return false;
}

QString ValidatorSame::genericErrorMessage() const
{
    Q_D(const ValidatorSame);

    QString ol = !d->otherLabel.isEmpty() ? d->otherLabel : d->otherField;

    return tr("The “%1” field must have the same value as the “%2” field.").arg(genericFieldName(), ol);
}

void ValidatorSame::setOtherField(const QString &otherField)
{
    Q_D(ValidatorSame);
    d->otherField = otherField;
}
