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
 * It uses <A HREF="http://docs.libmemcached.org">libmemcached</A> to connect to a pool of memcached servers
 * and to perform the caching operations. In order to build this plugin, the libmemcached development and header
 * files have to be present at build time.
 *
 * Basically all values are stored as QByteArray. So, to store simple types, simply convert them into a QByteArray
 * and vice versa on retrieval. For more complex or custom types you can use QDataStream to srialize them into
 * a QByteArray. For most methods in this plugin there are template functions for convenience that perform this
 * serialization. The requirement to use them is that the types to store and get provide stream operators for
 * QDataStream.
 *
 * \par Configuration
 * \parblock
 * The Memcached plugin can be configured in the cutelyst configuration file in the \a Cutelyst_Memcached_Plugin section.
 * It uses the same configuration strings as <A HREF="http://docs.libmemcached.org/libmemcached_configuration.html">libmemcached</A>
 * but in lowercase and without the dashes in front and for consistence \a - replaced by \a _ . So \c --BINARY-PROTOCOL will be
 * \c binary_protocol. To add servers and/or sockets use the \a servers configuration key. Servers can be added with name, port and
 * weight, separated by \c , - multiple servers are separated by a \c ; . To add sockets, use a full path as name. If no configuration
 * has been set or if the \a servers configuration key is empty, a default server at localhost on port 11211 will be used.
 *
 * Additional to the configuration options of libmemcached there are some plugin specific options:
 * \li \a compression - boolean value, enables compression of input values based on qCompress / zlib (default: disabled)
 * \li \a compression_level - integer value, the compression level used by qCompress (default: -1)
 * \li \a compression_threshold - integer value, the compression size threshold in bytes, only input values bigger than the threshold will be compressed (default: 100)
 * \endparblock
 *
 * \note If you want to use non-ASCII key names you have to enable the binary protocol.
 *
 * \par Configuration example
 *
 * \code{.ini}
 * [Cutelyst_Memcached_Plugin]
 * servers=cache.example.com,11211,2;/path/to/memcached.sock,1
 * binary_protocol=true
 * namespace=tritratrullala
 * \endcode
 *
 * \par Expiration times
 * Expiration times are set in seconds. If the value is bigger than 30 days, it is interpreted as a unix timestamp.
 *
 * \par Logging and return types
 * Messages from this plugin are logged to the logging category \a cutelyst.plugin.memcached. All methods provide
 * the possibility to specify a pointer to an MemcachedReturnType variable that can provide further information
 * about occured errors if methods return \c false or empty results.
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

    /**
     * Writes the \a value to the memcached server using \a key. If the \a key
     * already exists it will overwrite what is on the server. If the object
     * does not exist it will be written.
     *
     * @param[in] key key of object whose value to set
     * @param[in] value of type \a T of object to write to server
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return \c true on success; \c false otherwise
     */
    static bool set(const QString &key, const QByteArray &value, quint32 expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Writes the \a value of type \a T to the memcached server using \a key. If
     * the \a key already exists it will overwrite what is on the server. If the
     * object does not exist it will be written.
     *
     * Type \a T has to be serializable into a QByteArray using QDataStream.
     *
     * @param[in] key key of object whose value to set
     * @param[in] value value of object to write to server
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return \c true on success; \c false otherwise
     */
    template< typename T>
    static bool set(const QString &key, const T &value, quint32 expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Writes the \a value to the memcached server using \a key. If the \a key already
     * exists it will overwrite what is on the server. If the object does not exist it
     * will be written. This method is functionally equivalent to Memcached::set(),
     * except that the free-form \a groupKey can be used to map the \a key to a specific
     * server. This allows related items to be grouped together on a single server for
     * efficiency.
     *
     * @param[in] groupKey key that specifies the server to write to
     * @param[in] key key of object whose value to set
     * @param[in] value value of object to write to server
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return \c true on success; \c false otherwise
     */
    static bool setByKey(const QString &groupKey, const QString &key, const QByteArray &value, quint32 expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Writes the \a value of type \a T to the memcached server using \a key. If the \a key
     * already exists it will overwrite what is on the server. If the object does not exist
     * it will be written. This method is functionally equivalent to Memcached::set(), except
     * that the free-form \a groupKey can be used to map the \a key to a specific server.
     * This allows related items to be grouped together on a single server for efficiency.
     *
     * Type \a T has to be serializable into a QByteArray using QDataStream.
     *
     * @param[in] groupKey key that specifies the server to write to
     * @param[in] key key of object whose value to set
     * @param[in] value value of type \a T of object to write to server
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return \c true on success; \c false otherwise
     */
    template< typename T>
    static bool setByKey(const QString &groupKey, const QString &key, const T &value, quint32 expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Adds the \a value to the memcached server using \a key. If the object is found on the server
     * an error occurs and this method returns \c false, otherwise the value is stored.
     *
     * As this method returns \c false if the \a key has already been set on the server, you can
     * use the value of the \a returnType to determine the reason. Other than with other errors,
     * failing because of already existing \a key will not be logged.
     *
     * @param[in] key key of object whose value to add
     * @param[in] value value of object to add to server
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return \c true on success; \c false otherwise
     */
    static bool add(const QString &key, const QByteArray &value, quint32 expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Adds the \a value of type \a T to the memcached server using \a key. If the object is found
     * on the server an error occurs and this method returns \c false, otherwise the value is stored.
     *
     * Type \a T has to be serializable into a QByteArray using QDataStream.
     *
     * As this method returns \c false if the \a key has already been set on the server, you can use
     * the value of the \a returnType to determine the reason. Other than with other errors, failing
     * because of already existing \a key will not be logged.
     *
     * @param[in] key key of object whose value to add
     * @param[in] value value of type \a T of object to add to server
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[out] returnType optional pointer to MemcachedReturnType variable that takes the return type of the operation
     * @return \c true on success; \c false otherwise
     */
    template< typename T>
    static bool add(const QString &key, const T &value, quint32 expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Adds the \a value to the memcached server using \a key. If the object is found on the server an
     * error occurs and this method returns \c false, otherwise the value is stored. This method is
     * functionally equivalent to Memcached::add(), except that the free-form \a groupKey can be used
     * to map the \a key to a specific server. This allows related items to be grouped together on a
     * single server for efficiency.
     *
     * As this method returns \c false if the \a key has already been set on the server, you can use
     * the value of the \a returnType to determine the reason. Other than with other errors, failing
     * because of already existing \a key will not be logged.
     *
     * @param[in] groupKey key that specifies the server to write to
     * @param[in] key key of object whose value to add
     * @param[in] value value of object to add to server
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return \c true on success; \c false otherwise
     */
    static bool addByKey(const QString &groupKey, const QString &key, const QByteArray &value, quint32 expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Adds the \a value of type \a T to the memcached server using \a key. If the object is found on
     * the server an error occurs and this method returns \c false, otherwise the value is stored. This
     * method is functionally equivalent to Memcached::add(), except that the free-form \a groupKey can
     * be used to map the \a key to a specific server. This allows related items to be grouped together
     * on a single server for efficiency.
     *
     * Type \a T has to be serializable into a QByteArray using QDataStream.
     *
     * As this method returns \c false if the \a key has already been set on the server, you can use
     * the value of the \a returnType to determine the reason. Other than with other errors, failing
     * because of already existing \a key will not be logged.
     *
     * @param[in] groupKey key that specifies the server to write to
     * @param[in] key key of object whose value to add
     * @param[in] value value of object to add to server
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return \c true on success; \c false otherwise
     */
    template< typename T>
    static bool addByKey(const QString &groupKey, const QString &key, const T &value, quint32 expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Replaces the data of \a key on the server with \a value. If the \a key ist not found on the
     * server an error occures and \c false will be returned.
     *
     * As this method returns \c false if the \a key can not be found on the server, you can use
     * the value of the \a returnType to determine the reason. Other than with other errors, failing
     * because of not found \a key will not be logged.
     *
     * @param[in] key key of object whose value to replace
     * @param[in] value value to replace object on server with
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return \c true on success; \c false otherwise
     */
    static bool replace(const QString &key, const QByteArray &value, quint32 expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Replaces the data of \a key on the server with \a value of type \a T. If the \a key ist not
     * found on the server an error occures and \c false will be returned.
     *
     * Type \a T has to be serializable into a QByteArray using QDataStream.
     *
     * As this method returns \c false if the \a key can not be found on the server, you can use
     * the value of the \a returnType to determine the reason. Other than with other errors, failing
     * because of not found \a key will not be logged.
     *
     * @param[in] key key of object whose value to replace
     * @param[in] value value to replace object on server with
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return \c true on success; \c false otherwise
     */
    template< typename T>
    static bool replace(const QString &key, const T &value, quint32 expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Replaces the data of \a key on the server with \a value. If the \a key ist not found on the
     * server an error occures and \c false will be returned. This method is functionally equivalent
     * to Memcached::replace(), except that the free-form \a groupKey can be used to map the \a key
     * to a specific server. This allows related items to be grouped together on a single server for
     * efficiency.
     *
     * As this method returns \c false if the \a key can not be found on the server, you can use
     * the value of the \a returnType to determine the reason. Other than with other errors, failing
     * because of not found \a key will not be logged.
     *
     * @param[in] groupKey key that specifies the server to write to
     * @param[in] key key of object whose value to replace
     * @param[in] value value to replace object on server with
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return \c true on success; \c false otherwise
     */
    static bool replaceByKey(const QString &groupKey, const QString &key, const QByteArray &value, quint32 expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Replaces the data of \a key on the server with \a value of Type \a T. If the \a key ist not found
     * on the server an error occures and \c false will be returned. This method is functionally equivalent
     * to Memcached::replace(), except that the free-form \a groupKey can be used to map the \a key
     * to a specific server. This allows related items to be grouped together on a single server for
     * efficiency.
     *
     * Type \a T has to be serializable into a QByteArray using QDataStream.
     *
     * As this method returns \c false if the \a key can not be found on the server, you can use
     * the value of the \a returnType to determine the reason. Other than with other errors, failing
     * because of not found \a key will not be logged.
     *
     * @param[in] groupKey key that specifies the server to write to
     * @param[in] key key of object whose value to replace
     * @param[in] value value to replace object on server with
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return \c true on success; \c false otherwise
     */
    template< typename T>
    static bool replaceByKey(const QString &groupKey, const QString &key, const T &value, quint32 expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Fetch an individial value from the server identified by \a key. The returned QByteArray will
     * contain the data fetched from the server. If an error occured or if the \a key could not
     * be found, the returned QByteArray will be \c null. Use QByteArray::isNull() to check for this.
     *
     * As this method returns a \c null byte array if an error occured as well if the \a key could
     * not be found, you can use the value of the \a returnType to determine the reason. Other than with
     * other errors, failing because of not found \a key will not be logged.
     *
     * @param[in] key key of object whose value to fecth
     * @param[out] cas optional pointer to a quint32 variable that takes the CAS value
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return QByteArray containing the data fetched from the server; if an error occured or the \a key has not been found, this will be \c null.
     */
    static QByteArray get(const QString &key, quint64 *cas = nullptr, MemcachedReturnType *returnType = nullptr);

    /**
     * Fetch an individial value of type \a T from the server identified by \a key. The returned type \a T
     * will contain the data fetched from the server. If an error occured or if the \a key could not
     * be found, the returned type \a T will be default constructed.
     *
     * Type \a T has to be serializable into a QByteArray using QDataStream.
     *
     * As this method returns a default constructed type \a T if an error occured as well if the \a key
     * could not be found, you can use the value of the \a returnType to determine the reason.
     * Other than with other errors, failing because of not found \a key will not be logged.
     *
     * \par Usage example
     * \code{.cpp}
     * void MyController::index(Context *c)
     * {
     *     //...
     *
     *     QVariantList list = Memcached::get<QVariantList>(QStringLiteral("MyKey"));
     *
     *     //...
     * }
     * \endcode
     *
     * @param[in] key key of object whose value to fecth
     * @param[out] cas optional pointer to a quint32 variable that takes the CAS value
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return type T containing the data fetched from the server; if an error occured or the \a key has not been found, this will be a default constructed value
     */
    template< typename T>
    static T get(const QString &key, quint64 *cas = nullptr, MemcachedReturnType *returnType = nullptr);

    /**
     * Fetch an individial value from the server identified by \a key. The returned QByteArray will
     * contain the data fetched from the server. If an error occured or if the \a key could not
     * be found, the returned QByteArray will be \c null. Use QByteArray::isNull() to check for this.
     * This method behaves in a similar nature as Memcached::get(). The difference is that it takes
     * a \a groupKey that is used for determining which server an object was stored if key partitioning
     * was used for storage.
     *
     * As this method returns a \c null byte array if an error occured as well if the \a key could
     * not be found, you can use the value of the \a returnType to determine the reason. Other than with
     * other errors, failing because of not found \a key will not be logged.
     *
     * @param[in] groupKey key that specifies the server to fetch from
     * @param[in] key key of object whose value to fecth
     * @param[out] cas optional pointer to a quint32 variable that takes the CAS value
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return QByteArray containing the data fetched from the server; if an error occured or the \a key has not been found, this will be \c null.
     */
    static QByteArray getByKey(const QString &groupKey, const QString &key, quint64 *cas = nullptr, MemcachedReturnType *returnType = nullptr);

    /**
     * Fetch an individial value from the server identified by \a key. The returned type \a T will
     * contain the data fetched from the server. If an error occured or if the \a key could not
     * be found, the returned type \a T will be default constructed.
     * This method behaves in a similar nature as Memcached::get(). The difference is that it takes
     * a \a groupKey that is used for determining which server an object was stored if key partitioning
     * was used for storage.
     *
     * As this method returns a default constructed type \a T if an error occured as well if the \a key
     * could not be found, you can use the value of the \a returnType to determine the reason.
     * Other than with other errors, failing because of not found \a key will not be logged.
     *
     * \par Usage example
     * \code{.cpp}
     * void MyController::index(Context *c)
     * {
     *     //...
     *
     *     QVariantList list = Memcached::getByKey<QVariantList>(QStringLiteral("MyGroup"), QStringLiteral("MyKey"));
     *
     *     //...
     * }
     * \endcode
     *
     * @param[in] groupKey key that specifies the server to fetch from
     * @param[in] key key of object whose value to fecth
     * @param[out] cas optional pointer to a quint32 variable that takes the CAS value
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return type T containing the data fetched from the server; if an error occured or the \a key has not been found, this will be a default constructed value
     */
    template< typename T>
    static T getByKey(const QString &groupKey, const QString &key, quint64 *cas = nullptr, MemcachedReturnType *returnType = nullptr);

    /**
     * Deletes a particular \a key. \a Expiration works by placing the item into a delete queue,
     * which means that it won’t be possible to retrieve it by the “get” command. The “add”
     * and “replace” commands with this key will also fail (the “set” command will succeed,
     * however). After the time passes, the item is finally deleted from server memory.
     *
     * @param[in] key key of object to delete
     * @param[in] expiration expiration time in seconds
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return \c true on success; \c false otherwise
     */
    static bool remove(const QString &key, quint32 expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Directly deletes a particular \a key by setting the expiration time to \c 0.
     *
     * @param[in] key key of object to delete
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return \c true on success; \c false otherwise
     */
    static bool remove(const QString &key, MemcachedReturnType *returnType = nullptr);

    /**
     * Deletes a particular \a key in \a groupkey. This method behaves in a similar nature as
     * Memcached::remove(). The difference is that it takes a \a groupKey that is used for
     * determining which server an object was stored if key partitioning was used for storage.
     * \a Expiration works by placing the item into a delete queue,
     * which means that it won’t be possible to retrieve it by the “get” command. The “add”
     * and “replace” commands with this key will also fail (the “set” command will succeed,
     * however). After the time passes, the item is finally deleted from server memory.
     *
     * @param[in] groupKey key that specifies the server to delete from
     * @param[in] key key of object to delete
     * @param[in] expiration expiration time in seconds
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return \c true on success; \c false otherwise
     */
    static bool removeByKey(const QString &groupKey, const QString &key, quint32 expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Directly deletes a particular \a key in \a groupKey by setting the expiration time to \c 0.
     * This method behaves in a similar nature as Memcached::remove(). The difference is that it
     * takes a \a groupKey that is used for determining which server an object was stored if key
     * partitioning was used for storage.
     *
     * @param[in] groupKey key that specifies the server to delete from
     * @param[in] key key of object to delete
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return \c true on success; \c false otherwise
     */
    static bool removeByKey(const QString &groupKey, const QString &key, MemcachedReturnType *returnType = nullptr);

    /**
     * Checks if the \a key exists.
     * @param[in] key key to check
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return \c true if the key exists; \c false otherwise
     */
    static bool exist(const QString &key, MemcachedReturnType *returnType = nullptr);

    /**
     * Checks if the \a key exists in \a groupKey. This method behaves in a similar nature as
     * Memcached::exist(). The difference is that it takes a \a groupKey that is used for
     * determining which server an object was stored if key partitioning was used for storage.
     *
     * @param groupKey key that specifies the server to check on
     * @param key key to check
     * @param returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return \c true if the key exists; \c false otherwise
     */
    static bool existByKey(const QString &groupKey, const QString &key, MemcachedReturnType *returnType = nullptr);

    /**
     * Increments the value of \a key by \a offset. If there is a valid pointer to \a value, the incremented value
     * will be returned to it.
     *
     * @note Be aware that the memcached server does not detect overflow and underflow.
     *
     * @param[in] key key to increment
     * @param[in] offset offset for increment
     * @param[out] value optional pointer to a variable that takes the incremented value
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return \c true on success; \c false otherwise
     */
    static bool increment(const QString &key, uint32_t offset, uint64_t *value = nullptr, MemcachedReturnType *returnType = nullptr);

    /**
     * Increments the value of \a key in \a groupKey by \a offset. If there is a valid pointer to \a value, the
     * incremented value will be returned to it. This method behaves in a similar nature as
     * Memcached::increment(). The difference is that it takes a \a groupKey that is used for
     * determining which server an object was stored if key partitioning was used for storage.
     *
     * @note Be aware that the memcached server does not detect overflow and underflow.
     *
     * @param[in] groupKey key that specifies the server to increment the key on
     * @param[in] key key to increment
     * @param[in] offset offset for increment
     * @param[out] value optional pointer to a variable that takes the incremented value
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return \c true on success; \c false otherwise
     */
    static bool incrementByKey(const QString &groupKey, const QString &key, uint64_t offset, uint64_t *value = nullptr, MemcachedReturnType *returnType = nullptr);

    /**
     * Expiration time constant that can be used in the increment/decrement with initial methods.
     * \sa incrementWithInitial() incrementWithInitialByKey() decrementWithInitial() decrementWithInitialByKey()
     */
    static const time_t expirationNotAdd;

    /**
     * Increments the value of \a key by \a offset. If the object specified by \a key does not exist,
     * one of two things will happen: if the expiration value is Memcached::expirationNotAdd, the
     * operation will fail. For all other expiration values, the operation will succeed by seeding
     * the value for that \a key with a \a initial value to expire with the provided expiration time.
     * The flags will be set to zero. If there is a valid pointer to \a value, the created or incremented
     * value will be returned to it.
     *
     * @note This method will only work when using the binary protocol.
     *
     * @note Be aware that the memcached server does not detect overflow and underflow.
     *
     * @param[in] key key to increment or initialize
     * @param[in] offset offset for increment
     * @param[in] initial initial value if key does not exist
     * @param[in] expiration expiration time in seconds
     * @param[out] value optional pointer to a variable that takes the incremented or initialized value
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return \c true on success; \c false otherwise
     */
    static bool incrementWithInitial(const QString &key, uint64_t offset, uint64_t initial, time_t expiration, uint64_t *value = nullptr, MemcachedReturnType *returnType = nullptr);

    /**
     * Increments the value of \a key in \a groupKey by \a offset. If the object specified by \a key does not exist,
     * one of two things will happen: if the expiration value is Memcached::expirationNotAdd, the
     * operation will fail. For all other expiration values, the operation will succeed by seeding
     * the value for that \a key with a \a initial value to expire with the provided expiration time.
     * The flags will be set to zero. If there is a valid pointer to \a value, the created or incremented
     * value will be returned to it.
     *
     * This method behaves in a similar nature as Memcached::incrementWithInitial(). The difference is that
     * it takes a \a groupKey that is used for determining which server an object was stored if key
     * partitioning was used for storage.
     *
     * @note This method will only work when using the binary protocol.
     *
     * @note Be aware that the memcached server does not detect overflow and underflow.
     *
     * @param[in] groupKey key that specifies the server to increment the key on
     * @param[in] key key to increment or initialize
     * @param[in] offset offset for increment
     * @param[in] initial initial value if key does not exist
     * @param[in] expiration expiration time in seconds
     * @param[out] value optional pointer to a variable that takes the incremented or initialized value
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return \c true on success; \c false otherwise
     */
    static bool incrementWithInitialByKey(const QString &groupKey, const QString &key, uint64_t offset, uint64_t initial, time_t expiration, uint64_t *value = nullptr, MemcachedReturnType *returnType = nullptr);

protected:
    const QScopedPointer<MemcachedPrivate> d_ptr;

    /**
     * Reads the configuration and sets up the plugin.
     */
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
bool Memcached::replace(const QString &key, const T &value, quint32 expiration, MemcachedReturnType *returnType)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << value;
    return Memcached::replace(key, value, expiration, returnType);
}

template< typename T>
bool Memcached::replaceByKey(const QString &groupKey, const QString &key, const T &value, quint32 expiration, MemcachedReturnType *returnType)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << value;
    return Memcached::replaceByKey(groupKey, key, data, expiration, returnType);
}

template< typename T>
T Memcached::get(const QString &key, quint64 *cas, MemcachedReturnType *returnType)
{
    T retVal;
    QByteArray ba = Memcached::get(key, cas, returnType);
    if (!ba.isEmpty()) {
        QDataStream in(&ba, QIODevice::ReadOnly);
        in >> retVal;
    }
    return retVal;
}

template< typename T>
T Memcached::getByKey(const QString &groupKey, const QString &key, quint64 *cas, MemcachedReturnType *returnType)
{
    T retVal;
    QByteArray ba = Memcached::getByKey(groupKey, key, cas, returnType);
    if (!ba.isEmpty()) {
        QDataStream in(&ba, QIODevice::ReadOnly);
        in >> retVal;
    }
    return retVal;
}

}

#endif // CUTELYSTMEMCACHED_H
