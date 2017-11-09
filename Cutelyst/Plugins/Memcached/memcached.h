/*
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
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

#ifndef CUTELYSTMEMCACHED_H
#define CUTELYSTMEMCACHED_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/plugin.h>

#include <QDataStream>

namespace Cutelyst {

class MemcachedPrivate;

/*!
 * \brief Cutelyst Memcached plugin.
 *
 * The Memcached plugin for Cutelyst can be used to store, retrieve, delete and modify data on a
 * <A HREF="https://www.memcached.org/">memcached</A> general-purpose distributed memory caching system.
 * It uses <A HREF="http://docs.libmemcached.org">libmemcached</A>to connect to a pool of memcached servers
 * and to perform the caching operations. In order to build this plugin, the libmemcached development and header
 * files have to be present at build time.
 *
 * \par Configuration
 * The Memcached plugin can be configured in the cutelyst configuration file in the \a Cutelyst_Memcached_Plugin section.
 * It uses the same configuration strings as <A HREF="http://docs.libmemcached.org/libmemcached_configuration.html">libmemcached</A>
 * but in lowercase and without the dashes in front and for consistence \a - replaced by \a _ . So \c --BINARY-PROTOCOL will be
 * \c binary_protocol. To add servers and/or sockets use the \a servers configuration key. Servers can be added with name, port and
 * weight, separated by \c : - multiple servers are separated by a \c |. To add sockets, use a full path as name. If no configuration
 * has been set or if the \a servers configuration key is empty, a default server at localhost on port 11211 will be used.
 *
 * \code{.ini}
 * [Cutelyst_Memcached_Plugin]
 * servers=cache.example.com:11211:2|/path/to/memcached.sock:1
 * binary_protocol=true
 * namespace=tritratrullala
 * \endcode
 *
 * \note If you want to use non-ASCII key names you have to enable the binary protocol.
 *
 * \par Usage example
 *
 * \code{.cpp}
 * #include <Cutelyst/Plugins/Memcached/Memcached>
 *
 * bool MyCutelystApp::init()
 * {
 *     // other initialization stuff
 *     // ...
 *
 *     new Memcached(this);
 *
 *     // maybe more initialization stuff
 *     //...
 * }
 *
 * void MyController::index(Context *c)
 * {
 *     QVariantList myData = Memcached::get<QVariantList>(QStringLiteral("myKey"));
 *     if (myData.empty()) {
 *         QSqlQuery q = CPreparedSqlQuery(QStringLiteral("SELECT * FROM myTable"));
 *         if (q.exec()) {
 *             myData = Sql::queryToHashList(q);
 *             if (!myData.empty()) {
 *                 Memcached::set(QStringLiteral("myKey"), myData, 900);
 *             }
 *         }
 *     }
 *
 *     // present your data
 *     // ...
 * }
 * \endcode
 *
 * \since Cutelyst 1.11.0
 */
class CUTELYST_PLUGIN_MEMCACHED_EXPORT Memcached : public Plugin
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Memcached)
public:
    /*!
     * Constructs a new Memcached object with the given \a parent.
     */
    Memcached(Application *parent);

    /*!
     * Deconstructs the Memcached object.
     */
    virtual ~Memcached();

    /*!
     * Return types for memcached operations.
     */
    enum MemcachedReturnType {
        Success,                                /**< The request was successfully executed. */
        Failure,                                /**< A unknown failure has occurred in the server. */
        HostLookupFailure,                      /**< A DNS failure has occurred. */
        ConnectionFailure,                      /**< A unknown error has occured while trying to connect to a server. */
        WriteFailure,                           /**< An error has occured while trying to write to a server. */
        ReadFailure,                            /**< A read failure has occurred. */
        UnknownReadFailure,                     /**< An unknown read failure only occurs when either there is a bug in the server, or in rare cases where an ethernet nic is reporting dubious information. */
        ProtocolError,                          /**< An unknown error has occurred in the protocol. */
        ClientError,                            /**< An unknown client error has occured internally. */
        ServerError,                            /**< An unknown error has occurred in the server. */
        Error,                                  /**< A general error occured. */
        DataExists,                             /**< The data requested with the key given was found. */
        DataDoesNotExist,                       /**< The data requested with the key given was not found. */
        NotStored,                              /**< The request to store an object failed. */
        Stored,                                 /**< The requested object has been successfully stored on the server. */
        NotFound,                               /**< The object requested was not found. */
        MemoryAllocationFailure,                /**< An error has occurred while trying to allocate memory. */
        PartialRead,                            /**< The read was only partially successful. */
        SomeErrors,                             /**< A multi request has been made, and some underterminate number of errors have occurred. */
        NoServers,                              /**< No servers have been added to the memcached_st object. */
        End,                                    /**< The server has completed returning all of the objects requested. */
        Deleted,                                /**< The object requested by the key has been deleted. */
        Stat,                                   /**< A “stat” command has been returned in the protocol. */
        Errno,                                  /**< An error has occurred in the driver which has set errno. */
        NotSupported,                           /**< The given method is not supported in the server. */
        FetchNotFinished,                       /**< A request has been made, but the server has not finished the fetch of the last request. */
        Timeout,                                /**< Operation has timed out. */
        Buffered,                               /**< The request has been buffered. */
        BadKeyProvided,                         /**< The key provided is not a valid key. */
        InvalidHostProtocol,                    /**< The server you are connecting too has an invalid protocol. Most likely you are connecting to an older server that does not speak the binary protocol. */
        ServerMarkedDead,                       /**< The requested server has been marked dead. */
        UnknownStatKey,                         /**< The server you are communicating with has a stat key which has not be defined in the protocol. */
        E2Big,                                  /**< Item is too large for the server to store. */
        InvalidArguments,                       /**< The arguments supplied to the given function were not valid. */
        KeyTooBig,                              /**< The key that has been provided is too large for the given server. */
        AuthProblem,                            /**< An unknown issue has occured during authentication. */
        AuthFailure,                            /**< The credentials provided are not valid for this server. */
        AuthContinue,                           /**< Authentication has been paused. */
        ParseError,                             /**< An error has occurred while trying to parse the configuration string. You should use memparse to determine what the error was. */
        ParseUserError,                         /**< An error has occurred in parsing the configuration string. */
        Deprecated,                             /**< The method that was requested has been deprecated. */
        InProgress,
        ServerTemporaryDisabled,
        ServerMemoryAllocationFailure,
        MaximumReturn
    };
    Q_ENUM(MemcachedReturnType)

    static bool set(const QString &key, const QByteArray &value, quint32 expiration, MemcachedReturnType *returnType = nullptr);

    template< typename T>
    static bool set(const QString &key, const T &value, quint32 expiration, MemcachedReturnType *returnType = nullptr);

    static bool setByKey(const QString &groupKey, const QString &key, const QByteArray &value, quint32 expiration, MemcachedReturnType *returnType = nullptr);

    template< typename T>
    static bool setByKey(const QString &groupKey, const QString &key, const T &value, quint32 expiration, MemcachedReturnType *returnType = nullptr);

    static bool add(const QString &key, const QByteArray &value, quint32 expiration, MemcachedReturnType *returnType = nullptr);

    template< typename T>
    static bool add(const QString &key, const T &value, quint32 expiration, MemcachedReturnType *returnType = nullptr);

    static bool addByKey(const QString &groupKey, const QString &key, const QByteArray &value, quint32 expiration, MemcachedReturnType *returnType = nullptr);

    template< typename T>
    static bool addByKey(const QString &groupKey, const QString &key, const T &value, quint32 expiration, MemcachedReturnType *returnType = nullptr);

    static QByteArray get(const QString &key, MemcachedReturnType *returnType = nullptr);

    template< typename T>
    static T get(const QString &key, MemcachedReturnType *returnType = nullptr);

    static QByteArray getByKey(const QString &groupKey, const QString &key, MemcachedReturnType *returnType = nullptr);

    template< typename T>
    static T getByKey(const QString &groupKey, const QString &key, MemcachedReturnType *returnType = nullptr);

    static bool remove(const QString &key, quint32 expiration, MemcachedReturnType *returnType = nullptr);

    static bool remove(const QString &key, MemcachedReturnType *returnType = nullptr);

    static bool removeByKey(const QString &groupKey, const QString &key, quint32 expiration, MemcachedReturnType *returnType = nullptr);

    static bool removeByKey(const QString &groupKey, const QString &key, MemcachedReturnType *returnType = nullptr);

    static bool exist(const QString &key, MemcachedReturnType *returnType = nullptr);

    static bool existByKey(const QString &groupKey, const QString &key, MemcachedReturnType *returnType = nullptr);

protected:
    const QScopedPointer<MemcachedPrivate> d_ptr;

    virtual bool setup(Application *app) override;

private:
    Q_PRIVATE_SLOT(d_func(), void _q_postFork(Application*))
};

template< typename T>
bool Memcached::set(const QString &key, const T &value, quint32 expiration, MemcachedReturnType *returnType)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << value;
    return Memcached::set(key, data, expiration, returnType);
}

template< typename T>
bool Memcached::setByKey(const QString &groupKey, const QString &key, const T &value, quint32 expiration, MemcachedReturnType *returnType)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << value;
    return Memcached::setByKey(groupKey, key, data, expiration, returnType);
}

template< typename T>
bool Memcached::add(const QString &key, const T &value, quint32 expiration, MemcachedReturnType *returnType)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << value;
    return Memcached::add(key, data, expiration, returnType);
}

template< typename T>
bool Memcached::addByKey(const QString &groupKey, const QString &key, const T &value, quint32 expiration, MemcachedReturnType *returnType)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << value;
    return Memcached::addByKey(groupKey, key, data, expiration, returnType);
}

template< typename T>
T Memcached::get(const QString &key, MemcachedReturnType *returnType)
{
    T retVal;
    QByteArray ba = Memcached::get(key, returnType);
    if (!ba.isEmpty()) {
        QDataStream in(&ba, QIODevice::ReadOnly);
        in >> retVal;
    }
    return retVal;
}

template< typename T>
T Memcached::getByKey(const QString &groupKey, const QString &key, MemcachedReturnType *returnType)
{
    T retVal;
    QByteArray ba = Memcached::getByKey(groupKey, key, returnType);
    if (!ba.isEmpty()) {
        QDataStream in(&ba, QIODevice::ReadOnly);
        in >> retVal;
    }
    return retVal;
}

}

#endif // CUTELYSTMEMCACHED_H
