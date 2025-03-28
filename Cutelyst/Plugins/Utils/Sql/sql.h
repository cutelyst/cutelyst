/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CSQL_H
#define CSQL_H

#include <Cutelyst/Plugins/Utils/sql_export.h>
#include <Cutelyst/paramsmultimap.h>

#include <QtCore/QVariant>
#include <QtSql/QSqlDatabase>

namespace Cutelyst {

namespace Sql {

/**
 * @ingroup plugins-utils-sql
 * @headerfile "" <Cutelyst/Plugins/Utils/Sql>
 * @brief This is a helper class to create scoped transactions
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
     * Creates a %Transaction using the \a databaseName that you would pass to
     * Sql::databaseNameThread().
     */
    explicit Transaction(const QString &databaseName = {});

    /**
     * Creates a %Transaction using the \a database that you would get for example from
     * Sql::databaseThread().
     */
    explicit Transaction(const QSqlDatabase &database);

    /**
     * Destroys the %Transaction object and rolls back the transaction is still open and
     * commit() was not called.
     */
    ~Transaction();

    /**
     * Returns \c true if the database transaction is still running.
     */
    bool transaction() const;

    /**
     * Tries to commit the database transaction and returns \c true on success.
     */
    bool commit();

    /**
     * Manually rolls back the database transaction and returns \c true on success.
     */
    bool rollback();

private:
    QSqlDatabase m_db;
    bool m_transactionRunning;
};

/**
 * @ingroup plugins-utils-sql
 * Returns a QVariant hash for the first (if any) row
 * in the \a query object, it’s useful for creating
 * stash objects for say an user.
 */
CUTELYST_PLUGIN_UTILS_SQL_EXPORT QVariantHash queryToHashObject(QSqlQuery &query);

/**
 * @ingroup plugins-utils-sql
 * Returns a variant list of QVariant hashes for all the rows
 * in the \a query object, it’s useful for creating
 * stash objects for say a list of users.
 */
CUTELYST_PLUGIN_UTILS_SQL_EXPORT QVariantList queryToHashList(QSqlQuery &query);

/**
 * @ingroup plugins-utils-sql
 * Returns a QVariant map for the first (if any) row
 * in the \a query object, it’s useful for creating
 * stash objects for say an user.
 */
CUTELYST_PLUGIN_UTILS_SQL_EXPORT QVariantMap queryToMapObject(QSqlQuery &query);

/**
 * @ingroup plugins-utils-sql
 * Returns a QJsonObject for the first (if any) row in the \a query object.
 * Each column name is a key in the object.
 */
CUTELYST_PLUGIN_UTILS_SQL_EXPORT QJsonObject queryToJsonObject(QSqlQuery &query);

/**
 * @ingroup plugins-utils-sql
 * Returns a variant list of QVariant maps for all the rows
 * in the \a query object, it’s useful for creating
 * stash objects for say a list of users to be used by
 * JSON serializer.
 */
CUTELYST_PLUGIN_UTILS_SQL_EXPORT QVariantList queryToMapList(QSqlQuery &query);

/**
 * @ingroup plugins-utils-sql
 * Returns an array of QJsonObject objects for all the rows in the \a query object.
 */
CUTELYST_PLUGIN_UTILS_SQL_EXPORT QJsonArray queryToJsonObjectArray(QSqlQuery &query);

/**
 * @ingroup plugins-utils-sql
 * Returns a list of QVariantLists for all the rows in the \a query object, it’s fastest option to
 * pass to Cutelee view as columns are indexed by it’s position instead of a QString hash lookup.
 */
CUTELYST_PLUGIN_UTILS_SQL_EXPORT QVariantList queryToList(QSqlQuery &query);

/**
 * @ingroup plugins-utils-sql
 * Returns a QJsonArray of arrays for all the rows in the \a query object,
 * columns are indexed by it’s position instead of a QString map lookup.
 */
CUTELYST_PLUGIN_UTILS_SQL_EXPORT QJsonArray queryToJsonArray(QSqlQuery &query);

/**
 * @ingroup plugins-utils-sql
 * Returns a QVariantHash of QVariantHashes where the \a key parameter
 * is the field name in the \a query result. This is useful when you
 * want to access specific user by user name or user id.
 */
CUTELYST_PLUGIN_UTILS_SQL_EXPORT QVariantHash queryToIndexedHash(QSqlQuery &query,
                                                                 const QString &key);

/**
 * @ingroup plugins-utils-sql
 * Returns a QJsonObject of QJsonObject where the \a key parameter
 * is the field name in the \a query result. This is useful when you
 * want to access specific user by user name or user id.
 */
CUTELYST_PLUGIN_UTILS_SQL_EXPORT QJsonObject queryToIndexedJsonObject(QSqlQuery &query,
                                                                      const QString &key);

/**
 * @ingroup plugins-utils-sql
 * Bind \a params to the \a query, using the param name as
 * the placeholder prebended with ':', if \a htmlEscaped
 * is true the bound values will be the return of toHtmlEscaped()
 */
CUTELYST_PLUGIN_UTILS_SQL_EXPORT void bindParamsToQuery(QSqlQuery &query,
                                                        const Cutelyst::ParamsMultiMap &params,
                                                        bool htmlEscaped = true);

/**
 * @ingroup plugins-utils-sql
 * Returns a QSqlQuery object prepared with \a query using the database \a db.
 * This is specially useful to avoid pointers to prepared queries.
 *
 * For applications that use default QSqlDatabase() connection, not thread-safe:
 * QSqlQuery query = CPreparedSqlQuery("SELECT * FROM");
 *
 * For applications that do not use default QSqlDatabase(), the returned QSqlQuery
 * is a CUTELYST_PLUGIN_UTILS_SQL_EXPORT thread_local which glues QSqlQuery to the current thread
 * but you must have a per thread QSqlDatabase() connection for this to be completely safe:
 * QSqlQuery query = CPreparedSqlQueryForDatabase("SELECT * FROM", QSqlDatabase::data);
 *
 * The returned object is set to forward only and you must use a different
 * database connection and thread_local on CUTELYST_PLUGIN_UTILS_SQL_EXPORT objects to be
 * thread-safe.
 */
CUTELYST_PLUGIN_UTILS_SQL_EXPORT QSqlQuery preparedQuery(const QString &query,
                                                         QSqlDatabase db  = QSqlDatabase(),
                                                         bool forwardOnly = false);

/**
 * @ingroup plugins-utils-sql
 * Returns a string with as "dbName-threadNumber" to be used for connecting
 */
CUTELYST_PLUGIN_UTILS_SQL_EXPORT QString databaseNameThread(const QString &dbName = {});

/**
 * @ingroup plugins-utils-sql
 * Returns a QSqlDatabase named as "dbName-threadNumber" to be used for QSqlQuery
 */
CUTELYST_PLUGIN_UTILS_SQL_EXPORT QSqlDatabase databaseThread(const QString &dbName = {});

/**
 * @ingroup plugins-utils-sql
 * Returns a QSqlQuery object prepared with \a query using the \a dbName database
 * which will automatically get's in the form of databaseNameThread().
 * This is specially useful to avoid pointers to prepered queries.
 *
 * For applications that uses default QSqlDatabase() connection, not thread-safe:
 * QSqlQuery query = CPreparedSqlQuery("SELECT * FROM");
 *
 * For applications that do not use default QSqlDatabase(), the returned QSqlQuery
 * is a CUTELYST_PLUGIN_UTILS_SQL_EXPORT thread_local which glues QSqlQuery to the current thread
 * but you must have a per thread QSqlDatabase() connection for this to be completely safe:
 * QSqlQuery query = CPreparedSqlQueryForDatabase("SELECT * FROM", QSqlDatabase::data);
 *
 * The returned object is set to forward only and you must use a different
 * database connection and thread_local on CUTELYST_PLUGIN_UTILS_SQL_EXPORT objects to be
 * thread-safe.
 */
CUTELYST_PLUGIN_UTILS_SQL_EXPORT QSqlQuery preparedQueryThread(const QString &query,
                                                               const QString &dbName = {},
                                                               bool forwardOnly      = false);
} // namespace Sql

} // namespace Cutelyst

/**
 * @ingroup plugins-utils-sql
 * Constructs a static thread local QSqlQuery with query \a str on database \a db using
 * Cutelyst::Sql::preparedQuery() method. The created QSqlQuery object will be returned.
 */
#define CPreparedSqlQueryForDatabase(str, db) \
    ([]() -> QSqlQuery { \
        static thread_local QSqlQuery query_temp = Cutelyst::Sql::preparedQuery(str, db); \
        return query_temp; \
    }()) /**/

/**
 * @ingroup plugins-utils-sql
 * Constructs a static QSqlQuery with query \a str on the default database using
 * Cutelyst::Sql::preparedQuery(). The created QSqlQuery object will be returned.
 */
#define CPreparedSqlQuery(str) \
    ([]() -> QSqlQuery { \
        static QSqlQuery query_temp = Cutelyst::Sql::preparedQuery(str); \
        return query_temp; \
    }()) /**/

/**
 * @ingroup plugins-utils-sql
 * Constructs a static thread local QSqlQuery with query \a str on the database for the current
 * thread using Cutelyst::Sql::preparedQueryThread(). The created QSqlQuery object will be returned.
 */
#define CPreparedSqlQueryThread(str) \
    ([]() -> QSqlQuery { \
        static thread_local QSqlQuery query_temp = Cutelyst::Sql::preparedQueryThread(str); \
        return query_temp; \
    }()) /**/

/**
 * @ingroup plugins-utils-sql
 * Constructs a static thread local QSqlQuery with query \a str on database \a db using
 * Cutelyst::Sql::preparedQueryThread(). The created QSqlQuery object will be returned.
 */
#define CPreparedSqlQueryThreadForDB(str, db) \
    ([]() -> QSqlQuery { \
        static thread_local QSqlQuery query_temp = Cutelyst::Sql::preparedQueryThread(str, db); \
        return query_temp; \
    }()) /**/

/**
 * @ingroup plugins-utils-sql
 * Constructs a static thread local QSqlQuery with query \a str on database \a db using
 * Cutelyst::Sql::preparedQueryThread() with forwardOnly set to \c true. The created QSqlQuery
 * will be returned.
 */
#define CPreparedSqlQueryForDatabaseFO(str, db) \
    ([]() -> QSqlQuery { \
        static thread_local QSqlQuery query_temp = Cutelyst::Sql::preparedQuery(str, db, true); \
        return query_temp; \
    }()) /**/

/**
 * @ingroup plugins-utils-sql
 * Constructs a static QSqlQuery with query \a str on the default database using
 * Cutelyst::Sql::preparedQuery() with forwardOnly set to \c true. The crated query will be
 * returned.
 */
#define CPreparedSqlQueryFO(str) \
    ([]() -> QSqlQuery { \
        static QSqlQuery query_temp = Cutelyst::Sql::preparedQuery(str, QSqlDatabase(), true); \
        return query_temp; \
    }()) /**/

/**
 * @ingroup plugins-utils-sql
 * Constructs a static thread local QSqlQuery with query \a str on the database for the current
 * thread using Cutelyst::Sql::preparedQueryThread() with forwardOnly set to \c true. The created
 * QSqlQuery object will be returned.
 */
#define CPreparedSqlQueryThreadFO(str) \
    ([]() -> QSqlQuery { \
        static thread_local QSqlQuery query_temp = \
            Cutelyst::Sql::preparedQueryThread(str, {}, true); \
        return query_temp; \
    }()) /**/

/**
 * @ingroup plugins-utils-sql
 * Constructs a static thread local QSqlQuery with query \a str on database \a db using
 * Cutelyst::Sql::preparedQueryThread() with forwardOnly set to \c true. The created
 * QSqlQuery object will be returned.
 */
#define CPreparedSqlQueryThreadForDBFO(str, db) \
    ([]() -> QSqlQuery { \
        static thread_local QSqlQuery query_temp = \
            Cutelyst::Sql::preparedQueryThread(str, db, true); \
        return query_temp; \
    }()) /**/

#endif // CSQL_H
