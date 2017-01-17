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

#include "validatortime_p.h"
#include <QTime>

using namespace Cutelyst;

ValidatorTime::ValidatorTime(const QString &field, const QString &format, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorTimePrivate(field, format, label, customError))
{
}

ValidatorTime::ValidatorTime(ValidatorTimePrivate &dd) :
    ValidatorRule(dd)
{
}

ValidatorTime::~ValidatorTime()
{
}

QString ValidatorTime::validate() const
{
    QString result;

    Q_D(const ValidatorTime);

    const QString v = value();

    if (!v.isEmpty()) {
        const QTime date = d->extractTime(v, d->format);
        if (!date.isValid()) {
            result = validationError();
        }
    }

    return result;
}

QString ValidatorTime::genericValidationError() const
{
    QString error;

    Q_D(const ValidatorTime);

   if (!d->format.isEmpty()) {
       error = QStringLiteral("The data in the “%1” field can not be interpreted as time of this schema: %2").arg(fieldLabel(), d->format);
   } else {
       error = QStringLiteral("The data in the “%1” field can not be interpreted as time.").arg(fieldLabel());
   }

   return error;
}

void ValidatorTime::setFormat(const QString &format)
{
    Q_D(ValidatorTime);
    d->format = format;
}
