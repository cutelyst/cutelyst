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

#include "validatordate_p.h"
#include <QDate>

using namespace Cutelyst;

ValidatorDate::ValidatorDate(const QString &field, const QString &format, const QString &label, const QString &customError, QObject *parent) :
    ValidatorRule(*new ValidatorDatePrivate(field, format, label, customError), parent)
{

}


ValidatorDate::ValidatorDate(ValidatorDatePrivate &dd, QObject *parent) :
    ValidatorRule(dd, parent)
{

}


ValidatorDate::~ValidatorDate()
{

}



bool ValidatorDate::validate()
{
    Q_D(ValidatorDate);

    QString v = value();

    if (v.isEmpty()) {
        setValid(true);
        return true;
    }

    QDate date = d->extractDate(v, d->format);

    if (date.isValid()) {
        setValid(true);
        return true;
    }

    return false;
}



QString ValidatorDate::genericErrorMessage() const
{
    Q_D(const ValidatorDate);

    if (!d->format.isEmpty()) {
        return tr("The data in the “%1” field can not be interpreted as date of this schema: %2").arg(genericFieldName(), d->format);
    } else {
        return tr("The data in the “%1” field can not be interpreted as date.").arg(genericFieldName());
    }
}



void ValidatorDate::setFormat(const QString &format)
{
    Q_D(ValidatorDate);
    d->format = format;
}
