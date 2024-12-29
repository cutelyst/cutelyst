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

    /**
     * Returns all values for the current thread storage
     */
    static QVariantHash values();

    /**
     * Returns the value associated with the given key for the current thread storage
     */
    static QVariant value(const QString &key);

    /**
     * Stores a value for the given key, if value is different
     * than what is stored \sa valueChanged is emitted.
     */
    static void setValue(const QString &key, const QVariant &value);

    /**
     * Stores a default value for the given key, if there is already
     * a value associated with the given \p key nothing is done
     * else \sa valueChanged is emitted.
     */
    static void setDefaultValue(const QString &key, const QVariant &value);

Q_SIGNALS:
    void valueChanged(const QString &key, const QVariant &value);

private:
    void storeData(const QString &key, const QVariant &value);
    void storeDefaultData(const QString &key, const QVariant &value);

    QVariantHash m_data;
};

} // namespace Cutelyst
