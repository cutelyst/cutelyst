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

#include "validatorrequiredwithoutall_p.h"

using namespace Cutelyst;

ValidatorRequiredWithoutAll::ValidatorRequiredWithoutAll(const QString &field, const QStringList &otherFields, const QString &label, const QString &customError, QObject *parent) :
    ValidatorRule(*new ValidatorRequiredWithoutAllPrivate(field, otherFields, label, customError), parent)
{

}


ValidatorRequiredWithoutAll::ValidatorRequiredWithoutAll(ValidatorRequiredWithoutAllPrivate &dd, QObject *parent) :
    ValidatorRule(dd, parent)
{

}


ValidatorRequiredWithoutAll::~ValidatorRequiredWithoutAll()
{

}



bool ValidatorRequiredWithoutAll::validate()
{
    Q_D(ValidatorRequiredWithoutAll);

    if (d->otherFields.isEmpty()) {
        setValidationDataError(true);
        return false;
    }

    const QStringList ofc = d->otherFields;

    bool withoutAll = true;

    for (const QString &other : ofc) {
        if (d->parameters.contains(other)) {
            withoutAll = false;
            break;
        }
    }

    if (withoutAll) {
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



QString ValidatorRequiredWithoutAll::genericErrorMessage() const
{
    return tr("You must fill in the “%1” field.").arg(genericFieldName());
}



void ValidatorRequiredWithoutAll::setOtherFields(const QStringList &otherFields)
{
    Q_D(ValidatorRequiredWithoutAll);
    d->otherFields = otherFields;
}
