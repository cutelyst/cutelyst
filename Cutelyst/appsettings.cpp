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

void AppSettings::setValue(const QString &key, const QVariant &value)
{
    instance().storeData(key, value);
}

void AppSettings::setDefaultValue(const QString &key, const QVariant &value)
{
    instance().storeDefaultData(key, value);
}

QVariant AppSettings::value(const QString &key)
{
    return instance().m_data.value(key);
}

void AppSettings::storeData(const QString &key, const QVariant &value)
{
    auto it = m_data.find(key);
    if (it != m_data.end()) {
        if (it.value() == value) {
            return;
        }
        *it = value;
    } else {
        m_data.insert(key, value);
    }
    Q_EMIT valueChanged(key, value);
}

void AppSettings::storeDefaultData(const QString &key, const QVariant &value)
{
    auto it = m_data.find(key);
    if (it == m_data.end()) {
        m_data.insert(key, value);
        Q_EMIT valueChanged(key, value);
    }
}

#include "moc_appsettings.cpp"
