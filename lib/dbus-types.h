/*
 * Copyright (C) 2013 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef GENERIC_TYPES_H
#define GENERIC_TYPES_H

#include <QtCore/QVariantMap>
#include <QtDBus/QDBusArgument>
#include <QtCore/QVariantMap>

typedef QList<QByteArray> UDByteArrayList;
Q_DECLARE_METATYPE(UDByteArrayList)

typedef QMap<QString,QVariantMap> UDVariantMapMap;
Q_DECLARE_METATYPE(UDVariantMapMap)

typedef QMap<QDBusObjectPath, UDVariantMapMap> UDManagedObjects;
Q_DECLARE_METATYPE(UDManagedObjects)

typedef struct
{
    QString name;
    QVariantMap values;
} UDItem;
QDBusArgument &operator<<(QDBusArgument &argument, const UDItem &item);
const QDBusArgument &operator>>(const QDBusArgument &argument, UDItem &item);
Q_DECLARE_METATYPE(UDItem)

typedef QList<UDItem> UDItemList;
Q_DECLARE_METATYPE(UDItemList)

typedef struct
{
    quint8 id;
    QString name;
    quint16 flags;
    int value;
    int worst;
    int threshold;
    qint64 pretty;
    int pretty_unit;
    QVariantMap expansion;
} UDAttributes;
QDBusArgument &operator<<(QDBusArgument &argument, const UDAttributes &attributes);
const QDBusArgument &operator>>(const QDBusArgument &argument, UDAttributes &attributes);
Q_DECLARE_METATYPE(UDAttributes)

typedef struct
{
    QDBusObjectPath block;
    int slot;
    QStringList state;
    quint64 num_read_errors;
    QVariantMap expansion;
} UDActiveDevice;
QDBusArgument &operator<<(QDBusArgument &argument, const UDActiveDevice &activeDevice);
const QDBusArgument &operator>>(const QDBusArgument &argument, UDActiveDevice &activeDevice);
Q_DECLARE_METATYPE(UDActiveDevice)

#endif // GENERIC_TYPES_H
