/*
 * Copyright (C) 2015 Daniel Nicoletti <dantti12@gmail.com>
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

#include "utils.h"

#include <QTextStream>

using namespace Cutelyst;

QByteArray buildTableDivision(const QList<int> &columnsSize)
{
    QByteArray buffer;
    QTextStream out(&buffer, QIODevice::WriteOnly);
    for (int i = 0; i < columnsSize.size(); ++i) {
        if (i) {
            out << "+";
        } else {
            out << ".";
        }
        out << QByteArray().fill('-', columnsSize[i] + 2).data();
    }
    out << "." << endl;

    return buffer;
}

QByteArray Utils::buildTable(const QList<QStringList> &table, const QStringList &headers, const QString &title)
{
    QList<int> columnsSize;

    if (!headers.isEmpty()) {
        Q_FOREACH (const QString &header, headers) {
            columnsSize.append(header.size());
        }
    } else {
        Q_FOREACH (const QStringList &rows, table) {
            if (columnsSize.isEmpty()) {
                Q_FOREACH (const QString &row, rows) {
                    columnsSize.append(row.size());
                }
            } else if (rows.size() != columnsSize.size()) {
                qFatal("Incomplete table");
            }
        }
    }

    Q_FOREACH (const QStringList &row, table) {
        if (row.size() > columnsSize.size()) {
            qFatal("Incomplete table");
            break;
        }

        for (int i = 0; i < row.size(); ++i) {
            columnsSize[i] = qMax(columnsSize[i], row[i].size());
        }
    }

    // printing
    QByteArray buffer;
    QTextStream out(&buffer, QIODevice::WriteOnly);
    QByteArray div = buildTableDivision(columnsSize);

    if (!title.isEmpty()) {
        out << title << endl;
    }

    // Top line
    out << div;

    if (!headers.isEmpty()) {
        // header titles
        for (int i = 0; i < headers.size(); ++i) {
            out << "| " << headers[i].leftJustified(columnsSize[i]) << ' ';
        }
        out << '|' << endl;

        // header bottom line
        out << div;
    }

    Q_FOREACH (const QStringList &row, table) {
        // content table
        for (int i = 0; i < row.size(); ++i) {
            out << "| " << row[i].leftJustified(columnsSize[i]) << ' ';
        }
        out << '|' << endl;
    }

    // table bottom line
    out << div;

    return buffer;
}
