/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorresult_p.h"

#include <QJsonArray>
#include <QJsonValue>

using namespace Cutelyst;

ValidatorResult::ValidatorResult()
    : d(new ValidatorResultPrivate)
{
}

ValidatorResult::ValidatorResult(const ValidatorResult &other) noexcept = default;

ValidatorResult::ValidatorResult(ValidatorResult &&other) noexcept = default;

ValidatorResult &ValidatorResult::operator=(const ValidatorResult &other) noexcept = default;

ValidatorResult &ValidatorResult::operator=(ValidatorResult &&other) noexcept = default;

ValidatorResult::~ValidatorResult() noexcept = default;

bool ValidatorResult::isValid() const noexcept
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

QHash<QString, QStringList> ValidatorResult::errors() const noexcept
{
    return d->errors;
}

QStringList ValidatorResult::errors(const QString &field) const noexcept
{
    return d->errors.value(field);
}

bool ValidatorResult::hasErrors(const QString &field) const noexcept
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

QVariantHash ValidatorResult::values() const noexcept
{
    return d->values;
}

QVariant ValidatorResult::value(const QString &field) const noexcept
{
    return d->values.value(field);
}

void ValidatorResult::addValue(const QString &field, const QVariant &value)
{
    d->values.insert(field, value);
}

QVariantHash ValidatorResult::extras() const noexcept
{
    return d->extras;
}

QVariant ValidatorResult::extra(const QString &field) const noexcept
{
    return d->extras.value(field);
}

void ValidatorResult::addExtra(const QString &field, const QVariant &extra)
{
    d->extras.insert(field, extra);
}
