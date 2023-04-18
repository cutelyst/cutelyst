/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CSQL_H
#define CSQL_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/paramsmultimap.h>

#include <QtCore/QVariant>
#include <QtSql/QSqlDatabase>

namespace Cutelyst {

namespace Sql {

/**
 * @brief The Transaction class - This is a helper class to create scoped transactions
 *
 * This is a helper class to create scoped transactions, when you create a
 * local Transaction object it will automatically open a transaction, you
 * can check if the transaction was created successfully by calling transaction(),
 *
 * Once this object goes out of scope it will automatically rollback the transaction
 * if the transaction was open and commit() was not called. It helps you forgetting to
 * rollback a transaction in case of an error.
 */
class CUTELYST_PLUGIN_UTILS_SQL_EXPORT Transaction
{
public:
    /**
     * Creates a transaction using the database name that you would pass to Sql::databaseNameThread
     */
    explicit Transaction(const QString &databaseName = QString());
    Transaction(const QSqlDatabase &database);
    ~Transaction();

    bool transaction() const;
    bool commit();
    bool rollback();

private:
    QSqlDatabase m_db;
    bool m_transactionRunning;
};

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
CUTELYST_PLUGIN_UTILS_SQL_EXPORT QSqlQuery preparedQuery(const QString &query, QSqlDatabase db = QSqlDatabase(), bool forwardOnly = false);

/**
 * Returns a string with as "dbName-threadNumber" to be used for connecting
 */
CUTELYST_PLUGIN_UTILS_SQL_EXPORT QString databaseNameThread(const QString &dbName = QString());

/**
 * Returns a QSqlDatabase named as "dbName-threadNumber" to be used for QSqlQuery
 */
CUTELYST_PLUGIN_UTILS_SQL_EXPORT QSqlDatabase databaseThread(const QString &dbName = QString());

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
CUTELYST_PLUGIN_UTILS_SQL_EXPORT QSqlQuery preparedQueryThread(const QString &query, const QString &dbName = QString(), bool forwardOnly = false);
} // namespace Sql

} // namespace Cutelyst

#define CPreparedSqlQueryForDatabase(str, db) \
    ([]() -> QSqlQuery { \
        static thread_local QSqlQuery query_temp = \
            Cutelyst::Sql::preparedQuery(str, db); \
        return query_temp; \
    }()) /**/

#define CPreparedSqlQuery(str) \
    ([]() -> QSqlQuery { \
        static QSqlQuery query_temp = \
            Cutelyst::Sql::preparedQuery(str); \
        return query_temp; \
    }()) /**/

#define CPreparedSqlQueryThread(str) \
    ([]() -> QSqlQuery { \
        static thread_local QSqlQuery query_temp = \
            Cutelyst::Sql::preparedQueryThread(str); \
        return query_temp; \
    }()) /**/

#define CPreparedSqlQueryThreadForDB(str, db) \
    ([]() -> QSqlQuery { \
        static thread_local QSqlQuery query_temp = \
            Cutelyst::Sql::preparedQueryThread(str, db); \
        return query_temp; \
    }()) /**/

#define CPreparedSqlQueryForDatabaseFO(str, db) \
    ([]() -> QSqlQuery { \
        static thread_local QSqlQuery query_temp = \
            Cutelyst::Sql::preparedQuery(str, db, true); \
        return query_temp; \
    }()) /**/

#define CPreparedSqlQueryFO(str) \
    ([]() -> QSqlQuery { \
        static QSqlQuery query_temp = \
            Cutelyst::Sql::preparedQuery(str, QSqlDatabase(), true); \
        return query_temp; \
    }()) /**/

#define CPreparedSqlQueryThreadFO(str) \
    ([]() -> QSqlQuery { \
        static thread_local QSqlQuery query_temp = \
            Cutelyst::Sql::preparedQueryThread(str, QString(), true); \
        return query_temp; \
    }()) /**/

#define CPreparedSqlQueryThreadForDBFO(str, db) \
    ([]() -> QSqlQuery { \
        static thread_local QSqlQuery query_temp = \
            Cutelyst::Sql::preparedQueryThread(str, db, true); \
        return query_temp; \
    }()) /**/

#endif // CSQL_H
