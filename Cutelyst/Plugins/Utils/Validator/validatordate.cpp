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

ValidatorDate::ValidatorDate(const QString &field, const QString &format, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorDatePrivate(field, format, label, customError))
{
}

ValidatorDate::ValidatorDate(ValidatorDatePrivate &dd) :
    ValidatorRule(dd)
{
}

ValidatorDate::~ValidatorDate()
{
}


QString ValidatorDate::validate() const
{
    QString result;

    Q_D(const ValidatorDate);

    const QString v = value();

    if (!v.isEmpty()) {
        const QDate date = d->extractDate(v, d->format);

        if (!date.isValid()) {
            result = validationError();
        }
    }

    return result;
}

QString ValidatorDate::genericValidationError() const
{
    QString error;

    Q_D(const ValidatorDate);

    if (!d->format.isEmpty()) {
        error = QStringLiteral("The data in the “%1” field can not be interpreted as date of this schema: %2").arg(fieldLabel(), d->format);
    } else {
        error = QStringLiteral("The data in the “%1” field can not be interpreted as date.").arg(fieldLabel());
    }

    return error;
}

void ValidatorDate::setFormat(const QString &format)
{
    Q_D(ValidatorDate);
    d->format = format;
}
