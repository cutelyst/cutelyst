/*
 * Copyright (C) 2017-2018 Matthias Fehring <kontakt@buschmann23.de>
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
    QStringList fieldErrors = d->errors.value(field);
    fieldErrors.append(message);
    d->errors.insert(field, fieldErrors);
}

QStringList ValidatorResult::errorStrings() const
{
    QStringList strings;

    auto i = d->errors.constBegin();
    while (i != d->errors.constEnd()) {
        strings.append(i.value());
        ++i;
    }

    return strings;
}

QHash<QString, QStringList> ValidatorResult::errors() const
{
    return d->errors;
}

bool ValidatorResult::hasErrors(const QString &field) const
{
    return d->errors.contains(field);
}

QJsonObject ValidatorResult::errorsJsonObject() const
{
    QJsonObject json;

    if (!d->errors.empty()) {
        auto i = d->errors.constBegin();
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

QVariantHash ValidatorResult::values() const
{
    return d->values;
}

QVariant ValidatorResult::value(const QString &field) const
{
    return d->values.value(field);
}

void ValidatorResult::addValue(const QString &field, const QVariant &value)
{
    d->values.insert(field, value);
}

QVariantHash ValidatorResult::extras() const
{
    return d->extras;
}

QVariant ValidatorResult::extra(const QString &field) const
{
    return d->extras.value(field);
}

void ValidatorResult::addExtra(const QString &field, const QVariant &extra)
{
    d->extras.insert(field, extra);
}
