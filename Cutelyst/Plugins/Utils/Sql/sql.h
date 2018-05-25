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
#ifndef CSQL_H
#define CSQL_H

#include <QtCore/QVariant>
#include <QtSql/QSqlDatabase>

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/paramsmultimap.h>

namespace Cutelyst {

namespace Sql
{
    /**
     * Returns a QVariant hash for the first (if any) row
     * in the query object, it's useful for creating
     * stash objects for say an user
     */
    CUTELYST_PLUGIN_UTILS_SQL_EXPORT QVariantHash queryToHashObject(QSqlQuery &query);

    /**
     * Returns a variant list of QVariant hashes for all the rows
     * in the query object, it's useful for creating
     * stash objects for say a list of users
     */
    CUTELYST_PLUGIN_UTILS_SQL_EXPORT QVariantList queryToHashList(QSqlQuery &query);

    /**
     * Returns a QVariant map for the first (if any) row
     * in the query object, it's useful for creating
     * stash objects for say an user
     */
    CUTELYST_PLUGIN_UTILS_SQL_EXPORT QVariantMap queryToMapObject(QSqlQuery &query);

    /**
     * Returns a QJsonObject for the first (if any) row in the query object.
     * Each column name is a key in the object
     */
    CUTELYST_PLUGIN_UTILS_SQL_EXPORT QJsonObject queryToJsonObject(QSqlQuery &query);

    /**
     * Returns a variant list of QVariant maps for all the rows
     * in the query object, it's useful for creating
     * stash objects for say a list of users to be used by
     * JSON serializer
     */
    CUTELYST_PLUGIN_UTILS_SQL_EXPORT QVariantList queryToMapList(QSqlQuery &query);

    /**
     * Returns an array of QJsonObject objects for all the rows in the query object
     */
    CUTELYST_PLUGIN_UTILS_SQL_EXPORT QJsonArray queryToJsonObjectArray(QSqlQuery &query);

    /**
     * Returns a list of QVariantLists for all the rows in the query object, it's fastest option to
     * pass to Grantlee view as columns are indexed by it's position instead of a QString hash lookup.
     */
    CUTELYST_PLUGIN_UTILS_SQL_EXPORT QVariantList queryToList(QSqlQuery &query);

    /**
     * Returns a QJsonArray of arrays for all the rows in the query object,
     * columns are indexed by it's position instead of a QString map lookup.
     */
    CUTELYST_PLUGIN_UTILS_SQL_EXPORT QJsonArray queryToJsonArray(QSqlQuery &query);

    /**
     * Returns a QVariantHash of QVariantHashes where the key parameter
     * is the field name in the query result. This is useful when you
     * want to access specific user by user name or user id.
     */
    CUTELYST_PLUGIN_UTILS_SQL_EXPORT QVariantHash queryToIndexedHash(QSqlQuery &query, const QString &key);

    /**
     * Returns a QJsonObject of QJsonObject where the key parameter
     * is the field name in the query result. This is useful when you
     * want to access specific user by user name or user id.
     */
    CUTELYST_PLUGIN_UTILS_SQL_EXPORT QJsonObject queryToIndexedJsonObject(QSqlQuery &query, const QString &key);

    /**
     * Bind params to the query, using the param name as
     * the placeholder prebended with ':', if htmlEscaped
     * is true the bound values will be the return of toHtmlEscaped()
     */
    CUTELYST_PLUGIN_UTILS_SQL_EXPORT void bindParamsToQuery(QSqlQuery &query, const Cutelyst::ParamsMultiMap &params, bool htmlEscaped = true);

    /**
     * Returns a QSqlQuery object prepared with \pa query using the \pa db database
     * This is specially useful to avoid pointers to prepered queries.
     *
     * For applications that uses default QSqlDatabase() connection, not thread-safe:
     * QSqlQuery query = CPreparedSqlQuery("SELECT * FROM");
     *
     * For applications that do not use default QSqlDatabase(), the returned QSqlQuery
     * is a CUTELYST_PLUGIN_UTILS_SQL_EXPORT thread_local which glues QSqlQuery to the current thread but you must
     * have a per thread QSqlDatabase() connection for this to be completely safe:
     * QSqlQuery query = CPreparedSqlQueryForDatabase("SELECT * FROM", QSqlDatabase::data);
     *
     * The returned object is set to forward only and you must use a different
     * database connection and thread_local on CUTELYST_PLUGIN_UTILS_SQL_EXPORT objects to be thread-safe.
     */
    CUTELYST_PLUGIN_UTILS_SQL_EXPORT QSqlQuery preparedQuery(const QString &query, QSqlDatabase db = QSqlDatabase(), bool forwardOnly = true);

    /**
     * Returns a string with as "dbName-threadNumber" to be used for connecting
     */
    CUTELYST_PLUGIN_UTILS_SQL_EXPORT QString databaseNameThread(const QString &dbName = QString());

    /**
     * Returns a QSqlQuery object prepared with \pa query using the \pa dbName database
     * which will automatically get's in the form of databaseNameThread().
     * This is specially useful to avoid pointers to prepered queries.
     *
     * For applications that uses default QSqlDatabase() connection, not thread-safe:
     * QSqlQuery query = CPreparedSqlQuery("SELECT * FROM");
     *
     * For applications that do not use default QSqlDatabase(), the returned QSqlQuery
     * is a CUTELYST_PLUGIN_UTILS_SQL_EXPORT thread_local which glues QSqlQuery to the current thread but you must
     * have a per thread QSqlDatabase() connection for this to be completely safe:
     * QSqlQuery query = CPreparedSqlQueryForDatabase("SELECT * FROM", QSqlDatabase::data);
     *
     * The returned object is set to forward only and you must use a different
     * database connection and thread_local on CUTELYST_PLUGIN_UTILS_SQL_EXPORT objects to be thread-safe.
     */
    CUTELYST_PLUGIN_UTILS_SQL_EXPORT QSqlQuery preparedQueryThread(const QString &query, const QString &dbName = QString(), bool forwardOnly = true);
}

}

#  define CPreparedSqlQueryForDatabase(str, db) \
    ([]() -> QSqlQuery { \
        static thread_local QSqlQuery query_temp = \
            Cutelyst::Sql::preparedQuery(str, db); \
        return query_temp; \
    }()) \
    /**/

#  define CPreparedSqlQuery(str) \
    ([]() -> QSqlQuery { \
        static QSqlQuery query_temp = \
            Cutelyst::Sql::preparedQuery(str); \
        return query_temp; \
    }()) \
    /**/

#  define CPreparedSqlQueryThread(str) \
    ([]() -> QSqlQuery { \
        static thread_local QSqlQuery query_temp = \
            Cutelyst::Sql::preparedQueryThread(str); \
        return query_temp; \
    }()) \
    /**/

#  define CPreparedSqlQueryThreadForDB(str, db) \
    ([]() -> QSqlQuery { \
        static thread_local QSqlQuery query_temp = \
            Cutelyst::Sql::preparedQueryThread(str, db); \
        return query_temp; \
    }()) \
    /**/

#endif // CSQL_H
