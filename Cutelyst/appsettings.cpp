/*
 * SPDX-FileCopyrightText: (C) 2024 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "appsettings.h"

using namespace Cutelyst;

AppSettings &AppSettings::instance()
{
    static thread_local AppSettings settings;
    return settings;
}

QVariantHash AppSettings::values()
{
    return instance().m_data;
}

void AppSettings::setValues(const QVariantHash &data)
{
    instance().storeData(data);
}

QVariant AppSettings::value(const QString &key)
{
    return instance().m_data.value(key);
}

QVariant AppSettings::value(const QString &key, const QVariant &defaultValue)
{
    return instance().m_data.value(key, defaultValue);
}

void AppSettings::storeData(const QVariantHash &data)
{
    m_data = data;
    Q_EMIT valuesChanged(data);
}

#include "moc_appsettings.cpp"
