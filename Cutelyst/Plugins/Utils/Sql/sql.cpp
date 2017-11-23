/*
 * Copyright (C) 2015-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#include "sql.h"

#include <QtCore/QLoggingCategory>
#include <QThread>

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
    QVariantList ret;

    int columns = query.record().count();
    QStringList cols;
    for (int i = 0; i < columns; ++i) {
        cols.append(query.record().fieldName(i));
    }

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
    QVariantList ret;

    int columns = query.record().count();
    QStringList cols;
    for (int i = 0; i < columns; ++i) {
        cols.append(query.record().fieldName(i));
    }

    while (query.next()) {
        QVariantMap line;
        for (int i = 0; i < columns; ++i) {
            line.insert(cols.at(i), query.value(i));
        }
        ret.append(line);
    }
    return ret;
}

QVariantHash Sql::queryToIndexedHash(QSqlQuery &query, const QString &key)
{
    QVariantHash ret;
    QSqlRecord record = query.record();

    if (!record.contains(key)) {
        qCCritical(C_SQL) << "Field Name " << key <<
                             " not found in result set";
        return ret;
    }

    int columns = record.count();
    QStringList cols;

    for (int i = 0; i < columns; ++i) {
        cols.append(record.fieldName(i));
    }

    while (query.next()) {
        QVariantHash line;
        for (int i = 0; i < columns; ++i) {
            line.insertMulti(cols.at(i),query.value(i));
        }

        ret.insertMulti(query.value(key).toString(), line);
    }

    return ret;
}

void Sql::bindParamsToQuery(QSqlQuery &query, const Cutelyst::ParamsMultiMap &params, bool htmlEscaped)
{
    auto it = params.constBegin();
    if (htmlEscaped) {
        while (it != params.constEnd()) {
            if (it.value().isNull()) {
                query.bindValue(QLatin1Char(':') + it.key(), QVariant());
            } else {
                query.bindValue(QLatin1Char(':') + it.key(), it.value().toHtmlEscaped());
            }
            ++it;
        }
    } else {
        while (it != params.constEnd()) {
            if (it.value().isNull()) {
                query.bindValue(QLatin1Char(':') + it.key(), QVariant());
            } else {
                query.bindValue(QLatin1Char(':') + it.key(), it.value());
            }
            ++it;
        }
    }
}

QSqlQuery Sql::preparedQuery(const QString &query, QSqlDatabase db, bool forwardOnly)
{
    QSqlQuery sqlQuery(db);
    sqlQuery.setForwardOnly(forwardOnly);
    if (!sqlQuery.prepare(query)) {
        qCCritical(C_SQL) << "Failed to prepare query:" << query << sqlQuery.lastError().databaseText();
    }
    return sqlQuery;
}

QSqlQuery Sql::preparedQueryThread(const QString &query, const QString &dbName, bool forwardOnly)
{
    QSqlQuery sqlQuery(QSqlDatabase::database(databaseNameThread(dbName)));
    sqlQuery.setForwardOnly(forwardOnly);
    if (!sqlQuery.prepare(query)) {
        qCCritical(C_SQL) << "Failed to prepare query:" << query << sqlQuery.lastError().databaseText();
    }
    return sqlQuery;
}

QString Sql::databaseNameThread(const QString &dbName)
{
    return dbName + QLatin1Char('-') + QThread::currentThread()->objectName();
}
