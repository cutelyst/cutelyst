/*
 * SPDX-FileCopyrightText: (C) 2015-2023 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <Cutelyst/ParamsMultiMap>
#include <Cutelyst/cutelyst_global.h>

#include <QtCore/QStringList>

namespace Cutelyst {

namespace Utils {
CUTELYST_LIBRARY QByteArray buildTable(const QVector<QStringList> &table,
                                       const QStringList &headers = {},
                                       const QString &title       = {});

CUTELYST_LIBRARY QString decodePercentEncoding(QString *s);

CUTELYST_LIBRARY ParamsMultiMap decodePercentEncoding(char *data, int len);

CUTELYST_LIBRARY QString decodePercentEncoding(QByteArray *ba);
} // namespace Utils

} // namespace Cutelyst
