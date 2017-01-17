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

ValidatorDateTime::ValidatorDateTime(const QString &field, const QString &format, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorDateTimePrivate(field, format, label, customError))
{
}

ValidatorDateTime::ValidatorDateTime(ValidatorDateTimePrivate &dd) :
    ValidatorRule(dd)
{
}


ValidatorDateTime::~ValidatorDateTime()
{
}

QString ValidatorDateTime::validate() const
{
    QString result;

    Q_D(const ValidatorDateTime);

    const QString v = value().trimmed();

    if (!v.isEmpty()) {
        const QDateTime dt = d->extractDateTime(v, d->format);

        if (!dt.isValid()) {
            result = validationError();
        }
    }

    return result;
}

QString ValidatorDateTime::genericValidationError() const
{
    QString error;

    Q_D(const ValidatorDateTime);

    if (!d->format.isEmpty()) {
        error = QStringLiteral("The data in the “%1” field can not be interpreted as date and time of this schema: %2").arg(fieldLabel(), d->format);
    } else {
        error = QStringLiteral("The data in the “%1” field can not be interpreted as date and time.").arg(fieldLabel());
    }

    return error;
}

void ValidatorDateTime::setFormat(const QString &format)
{
    Q_D(ValidatorDateTime);
    d->format = format;
}
