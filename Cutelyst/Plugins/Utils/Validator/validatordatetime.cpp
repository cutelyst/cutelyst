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

#include "validatordatetime_p.h"
#include <QDateTime>

using namespace Cutelyst;

ValidatorDateTime::ValidatorDateTime(const QString &field, const QString &format, const QString &label, const QString &customError, QObject *parent) :
    ValidatorRule(*new ValidatorDateTimePrivate(field, format, label, customError), parent)
{

}


ValidatorDateTime::ValidatorDateTime(ValidatorDateTimePrivate &dd, QObject *parent) :
    ValidatorRule(dd, parent)
{

}


ValidatorDateTime::~ValidatorDateTime()
{

}



bool ValidatorDateTime::validate()
{
    Q_D(ValidatorDateTime);

    QString v = value().trimmed();

    if (v.isEmpty()) {
        setValid(true);
        return true;
    }

    QDateTime dt = d->extractDateTime(v, d->format);

    if (dt.isValid()) {
        setValid(true);
        return true;
    }

    return false;
}



QString ValidatorDateTime::genericErrorMessage() const
{
    Q_D(const ValidatorDateTime);

    if (!d->format.isEmpty()) {
        return tr("The data in the “%1” field can not be interpreted as date and time of this schema: %2").arg(genericFieldName(), d->format);
    } else {
        return tr("The data in the “%1” field can not be interpreted as date and time.").arg(genericFieldName());
    }
}


void ValidatorDateTime::setFormat(const QString &format)
{
    Q_D(ValidatorDateTime);
    d->format = format;
}
