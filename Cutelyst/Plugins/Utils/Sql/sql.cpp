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

#include "sql.h"

#include <QtCore/QLoggingCategory>

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>

Q_LOGGING_CATEGORY(C_SQL, "cutelyst.utils.sql")

using namespace Cutelyst;

QVariantHash Sql::queryToHashObject(QSqlQuery &query)
{
    QVariantHash ret;
    if (query.next()) {
        int columns = query.record().count();
        for (int i = 0; i < columns; ++i) {
            ret.insert(query.record().fieldName(i), query.value(i));
        }
    }
    return ret;
}

QVariantList Sql::queryToHashList(QSqlQuery &query)
{
    int columns = query.record().count();
    QStringList cols;
    for (int i = 0; i < columns; ++i) {
        cols.append(query.record().fieldName(i));
    }

    QVariantList ret;
    while (query.next()) {
        QVariantHash line;
        for (int i = 0; i < columns; ++i) {
            line.insert(cols.at(i), query.value(i));
        }
        ret.append(line);
    }
    return ret;
}

QVariantMap Sql::queryToMapObject(QSqlQuery &query)
{
    QVariantMap ret;
    if (query.next()) {
        int columns = query.record().count();
        for (int i = 0; i < columns; ++i) {
            ret.insert(query.record().fieldName(i), query.value(i));
        }
    }
    return ret;
}

QVariantList Sql::queryToMapList(QSqlQuery &query)
{
    int columns = query.record().count();
    QStringList cols;
    for (int i = 0; i < columns; ++i) {
        cols.append(query.record().fieldName(i));
    }

    QVariantList ret;
    while (query.next()) {
        QVariantMap line;
        for (int i = 0; i < columns; ++i) {
            line.insert(cols.at(i), query.value(i));
        }
        ret.append(line);
    }
    return ret;
}

void Sql::bindParamsToQuery(QSqlQuery &query, const Cutelyst::ParamsMultiMap &params, bool htmlEscaped)
{
    auto it = params.constBegin();
    if (htmlEscaped) {
        while (it != params.constEnd()) {
            if (it.value().isNull()) {
                query.bindValue(QLatin1Char(':') % it.key(), QVariant());
            } else {
                query.bindValue(QLatin1Char(':') % it.key(), it.value().toHtmlEscaped());
            }
            ++it;
        }
    } else {
        while (it != params.constEnd()) {
            if (it.value().isNull()) {
                query.bindValue(QLatin1Char(':') % it.key(), QVariant());
            } else {
                query.bindValue(QLatin1Char(':') % it.key(), it.value());
            }
            ++it;
        }
    }
}

QSqlQuery Sql::preparedQuery(const QString &query, QSqlDatabase db)
{
    QSqlQuery sqlQuery(db);
    if (!sqlQuery.prepare(query)) {
        qCCritical(C_SQL) << "Failed to prepare query:" << query << sqlQuery.lastError().databaseText();
    }
    return sqlQuery;
}
