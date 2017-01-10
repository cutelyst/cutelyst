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

#include "validatordifferent_p.h"

using namespace Cutelyst;

ValidatorDifferent::ValidatorDifferent(const QString &field, const QString &other, const QString &label, const QString &otherLabel, const QString &customError, QObject *parent) :
    ValidatorRule(*new ValidatorDifferentPrivate(field, other, label, otherLabel, customError), parent)
{
}

ValidatorDifferent::ValidatorDifferent(ValidatorDifferentPrivate &dd, QObject *parent) :
    ValidatorRule(dd, parent)
{
}

ValidatorDifferent::~ValidatorDifferent()
{
}

bool ValidatorDifferent::validate()
{
    Q_D(ValidatorDifferent);

    QString v = value();

    if (v.isEmpty()) {
        setValid(true);
        return true;
    }

    QString o = d->parameters.value(d->otherField).trimmed();

    if (v != o) {
        setValid(true);
        return true;
    }


    return false;
}

QString ValidatorDifferent::genericErrorMessage() const
{
    Q_D(const ValidatorDifferent);

    QString ogn = !d->otherLabel.isEmpty() ? d->otherLabel : d->otherField;

    return tr("The value in the “%1” field has to be different from the value in the “%2” field.").arg(genericFieldName(), ogn);
}

void ValidatorDifferent::setOtherField(const QString &otherField)
{
    Q_D(ValidatorDifferent);
    d->otherField = otherField;
}

void ValidatorDifferent::setOtherLabel(const QString &otherLabel)
{
    Q_D(ValidatorDifferent);
    d->otherLabel = otherLabel;
}
