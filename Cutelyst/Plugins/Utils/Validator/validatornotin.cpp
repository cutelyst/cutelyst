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

ValidatorNotIn::ValidatorNotIn(const QString &field, const QStringList &values, const QString &label, const QString &customError, QObject *parent) :
    ValidatorRule(*new ValidatorNotInPrivate(field, values, label, customError), parent)
{

}


ValidatorNotIn::ValidatorNotIn(ValidatorNotInPrivate &dd, QObject *parent) :
    ValidatorRule(dd, parent)
{

}


ValidatorNotIn::~ValidatorNotIn()
{

}



bool ValidatorNotIn::validate()
{
    Q_D(ValidatorNotIn);

    if (d->values.isEmpty()) {
        setValidationDataError(true);
        return false;
    }

    QString v = value();

    if (v.isEmpty()) {
        setValid(true);
        return true;
    }

    if (!d->values.contains(v)) {
        setValid(true);
        return true;
    }

    return false;
}



QString ValidatorNotIn::genericErrorMessage() const
{
    return tr("The value in the “%1“ field is not allowed.").arg(genericFieldName());
}



void ValidatorNotIn::setValues(const QStringList &values)
{
    Q_D(ValidatorNotIn);
    d->values = values;
}
