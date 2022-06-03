/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PARAMSMULTIMAP_H
#define PARAMSMULTIMAP_H

#include <QMultiMap>
#include <QString>
#include <QMetaType>

namespace Cutelyst {

/**
 * ParamsMultiMap is a QMultiMap used to store
 * request parameters with keys containing
 * multiple values.
 *
 * Since QMultiMap::values() return them in the
 * QMultiMap::insert() order they perfectly
 * fit our body or query parameters need.
 */
using ParamsMultiMap = QMultiMap<QString, QString>;

}

Q_DECLARE_METATYPE(Cutelyst::ParamsMultiMap)

#endif // PARAMSMULTIMAP_H
