/*
 * SPDX-FileCopyrightText: (C) 2024 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <Cutelyst/cutelyst_export.h>

#include <QObject>
#include <QVariantHash>

namespace Cutelyst {

/**
 * @brief The AppSettings class
 *
 * A thread local storage for dynamic application settings.
 *
 * Settings can be set and easily retrieved using the static methods,
 * and once new values are set \sa valuesChanged signal is emitted.
 */
class CUTELYST_EXPORT AppSettings : public QObject
{
    Q_OBJECT
public:
    static AppSettings &instance();

    static QVariantHash values();
    static void setValues(const QVariantHash &data);

    static QVariant value(const QString &key);
    static QVariant value(const QString &key, const QVariant &defaultValue);

Q_SIGNALS:
    void valuesChanged(const QVariantHash &values);

private:
    void storeData(const QVariantHash &data);

    QVariantHash m_data;
};

} // namespace Cutelyst
