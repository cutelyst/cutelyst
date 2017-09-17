/*
 * Copyright (C) 2015-2016 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef UTILS_H
#define UTILS_H

#include <QtCore/QStringList>

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

namespace Utils {
    CUTELYST_LIBRARY QByteArray buildTable(const QVector<QStringList> &table, const QStringList &headers = QStringList(), const QString &title = QString());

    CUTELYST_LIBRARY QString decodePercentEncoding(QString *s);

    CUTELYST_LIBRARY QString decodePercentEncoding(QByteArray *ba);
}

}

#endif // UTILS_H
