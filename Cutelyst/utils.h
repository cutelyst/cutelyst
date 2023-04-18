/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef UTILS_H
#define UTILS_H

#include <Cutelyst/ParamsMultiMap>
#include <Cutelyst/cutelyst_global.h>

#include <QtCore/QStringList>

namespace Cutelyst {

namespace Utils {
CUTELYST_LIBRARY QByteArray buildTable(const QVector<QStringList> &table, const QStringList &headers = QStringList(), const QString &title = QString());

CUTELYST_LIBRARY QString decodePercentEncoding(QString *s);

CUTELYST_LIBRARY ParamsMultiMap decodePercentEncoding(char *data, int len);

CUTELYST_LIBRARY QString decodePercentEncoding(QByteArray *ba);
} // namespace Utils

} // namespace Cutelyst

#endif // UTILS_H
