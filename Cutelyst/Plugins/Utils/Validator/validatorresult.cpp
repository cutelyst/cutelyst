/*
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "validatorresult_p.h"
#include <QJsonValue>
#include <QJsonArray>

using namespace Cutelyst;

ValidatorResult::ValidatorResult() :
    d(new ValidatorResultPrivate)
{
}

ValidatorResult::ValidatorResult(const ValidatorResult &other) :
    d(other.d)
{
}

ValidatorResult& ValidatorResult::operator =(const ValidatorResult &other)
{
    d = other.d;
    return *this;
}

ValidatorResult::~ValidatorResult()
{
}

bool ValidatorResult::isValid() const
{
    return d->errors.empty();
}

void ValidatorResult::addError(const QString &field, const QString &message)
{
    d->errorStrings.append(message);
    QStringList fieldErrors = d->errors.value(field);
    fieldErrors.append(message);
    d->errors.insert(field, fieldErrors);
}

QStringList ValidatorResult::errorStrings() const
{
    return d->errorStrings;
}

QHash<QString, QStringList> ValidatorResult::errors() const
{
    return d->errors;
}

QJsonObject ValidatorResult::errorsJsonObject() const
{
    QJsonObject json;

    if (!d->errors.empty()) {
        QHash<QString,QStringList>::const_iterator i = d->errors.constBegin();
        while (i != d->errors.constEnd()) {
            json.insert(i.key(), QJsonValue(QJsonArray::fromStringList(i.value())));
            ++i;
        }
    }

    return json;
}

QStringList ValidatorResult::failedFields() const
{
    return QStringList(d->errors.keys());
}
