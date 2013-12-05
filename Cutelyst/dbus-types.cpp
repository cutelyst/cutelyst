/*
Copyright 2011 Ilia Kats <ilia-kats@gmx.net>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dbus-types.h"

QDBusArgument &operator<<(QDBusArgument &argument, const UDItem &item)
{
    argument.beginStructure();
    argument << item.name << item.values;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, UDItem &item)
{
    argument.beginStructure();
    argument >> item.name >> item.values;
    argument.endStructure();
    return argument;
}


QDBusArgument &operator<<(QDBusArgument &argument, const UDAttributes &attributes)
{
    argument.beginStructure();
    argument << attributes.id
             << attributes.name
             << attributes.flags
             << attributes.value
             << attributes.worst
             << attributes.threshold
             << attributes.pretty
             << attributes.pretty_unit
             << attributes.expansion;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, UDAttributes &attributes)
{
    argument.beginStructure();
    argument >> attributes.id
             >> attributes.name
             >> attributes.flags
             >> attributes.value
             >> attributes.worst
             >> attributes.threshold
             >> attributes.pretty
             >> attributes.pretty_unit
             >> attributes.expansion;
    argument.endStructure();
    return argument;
}


QDBusArgument &operator<<(QDBusArgument &argument, const UDActiveDevice &activeDevice)
{
    argument.beginStructure();
    argument << activeDevice.block
             << activeDevice.slot
             << activeDevice.state
             << activeDevice.num_read_errors
             << activeDevice.expansion;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, UDActiveDevice &activeDevice)
{
    argument.beginStructure();
    argument >> activeDevice.block
             >> activeDevice.slot
             >> activeDevice.state
             >> activeDevice.num_read_errors
             >> activeDevice.expansion;
    argument.endStructure();
    return argument;
}
