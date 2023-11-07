/*
 * SPDX-FileCopyrightText: (C) 2014-2023 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <QMetaType>
#include <QMultiMap>
#include <QString>

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

} // namespace Cutelyst

Q_DECLARE_METATYPE(Cutelyst::ParamsMultiMap)
