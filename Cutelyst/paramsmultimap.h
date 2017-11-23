/*
 * Copyright (C) 2014-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef PARAMSMULTIMAP_H
#define PARAMSMULTIMAP_H

#include <QMap>
#include <QString>
#include <QMetaType>

namespace Cutelyst {

/**
 * ParamsMultiMap is a QMap used to store
 * request parameters with keys containing
 * multiple values.
 *
 * Since QMap::values() return them in the
 * QMap::insertMulti() order they perfectly
 * fit our body or query parameters need.
 */
typedef QMap<QString, QString> ParamsMultiMap;

}

Q_DECLARE_METATYPE(Cutelyst::ParamsMultiMap)

#endif // PARAMSMULTIMAP_H
