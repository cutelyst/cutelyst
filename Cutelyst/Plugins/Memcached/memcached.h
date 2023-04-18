/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CUTELYSTMEMCACHED_H
#define CUTELYSTMEMCACHED_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/plugin.h>

#include <QDataStream>
#include <QVersionNumber>

namespace Cutelyst {

class Context;
class MemcachedPrivate;

/**
 * @brief %Cutelyst %Memcached plugin.
 *
 * The %Memcached plugin for %Cutelyst can be used to store, retrieve, delete and modify data on a
 * <A HREF="https://www.memcached.org/">memcached</A> general-purpose distributed memory caching system.
 * It uses <A HREF="http://docs.libmemcached.org">libmemcached</A> to connect to a pool of memcached servers
 * and to perform the caching operations. In order to build this plugin, the libmemcached development and header
 * files have to be present at build time.
 *
 * Basically all values are stored as QByteArray. So, to store simple types, simply convert them into a QByteArray
 * and vice versa on retrieval. For more complex or custom types you can use QDataStream to serialize them into
 * a QByteArray. For most methods in this plugin there are template functions for convenience that perform this
 * serialization. The requirement to use them is that the types to store and get provide stream operators for
 * QDataStream.
 *
 * <H3>Configuration</h3>

 * The %Memcached plugin can be configured in the cutelyst configuration file in the @c Cutelyst_Memcached_Plugin section.
 * It uses the same configuration strings as <A HREF="http://docs.libmemcached.org/libmemcached_configuration.html">libmemcached</A>
 * but in lowercase and without the dashes in front and for consistence @a - replaced by @a _ . So @c --BINARY-PROTOCOL will be
 * @c binary_protocol. To add servers and/or sockets use the @a servers configuration key. Servers can be added with name, port and
 * weight, separated by @c , - multiple servers are separated by a @c ; . To add sockets, use a full path as name. If no configuration
 * has been set or if the @a servers configuration key is empty, a default server at localhost on port 11211 will be used.
 *
 * Additional to the <A HREF="http://docs.libmemcached.org/libmemcached_configuration.html">configuration options of libmemcached</A> there are some plugin specific options:
 * @li @a compression - boolean value, enables compression of input values based on qCompress / zlib (default: disabled)
 * @li @a compression_level - integer value, the compression level used by qCompress (default: -1)
 * @li @a compression_threshold - integer value, the compression size threshold in bytes, only input values bigger than the threshold will be compressed (default: 100)
 * @li @a encryption_key - string value, if set and not empty, AES encryption will be enabled (default: empty)
 * @li @a sasl_user - string value, if set and not empty, SASL authentication will be used - note that SASL support has to be enabled when building libmemcached (default: empty)
 * @li @a sasl_password - string value, if set and not empty, SASL authentication will be used (default: empty)
 *
 * @note If you want to use non-ASCII key names you have to enable the binary protocol.
 *
 * To set default values directly in your application, use setDefaultConfig(). Configuration values that can not be found
 * in the Cutelyst configuration file will be looked up for default values in that QVariantMap.
 *
 * <H4>Configuration example</h4>
 *
 * @code{.ini}
 * [Cutelyst_Memcached_Plugin]
 * servers=cache.example.com,11211,2;/path/to/memcached.sock,1
 * binary_protocol=true
 * namespace=foobar
 * @endcode
 *
 * <H3>Expiration times</H3>
 *
 * Expiration times are set in seconds. If the value is bigger than 30 days, it is interpreted as a unix timestamp.
 *
 * <H3>Logging and return types</H3>
 * Messages from this plugin are logged to the logging category @a cutelyst.plugin.memcached. All methods provide
 * the possibility to specify a pointer to a MemcachedReturnType variable that can provide further information
 * about occurred errors if methods return @c false or empty results.
 *
 * <H3>Usage example</H3>
 *
 * @code{.cpp}
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
 * @endcode
 *
 * <H3>Build requirements</H3>
 *
 * To build this plugin you need the development and header files for <A HREF="http://libmemcached.org">libmemcached</A>
 * and run cmake with <CODE>-DPLUGIN_MEMCACHED:BOOL=ON</CODE>.
 *
 * <H4>Unit test</H4>
 *
 * Enabling the build of the %Memcached plugin will also enable the unit tests for this plugin. By default, the unit test
 * will start its own memcached instance. Alternatively you can set the @c CUTELYST_MEMCACHED_TEST_SERVERS environment
 * variable in your build environment to define different servers that you have to start by yourself. The syntax is the
 * same as for adding servers in the configuration file. If you have for examble two servers, one on default location and
 * another one on a unix socket, export the following environment variable befor running the tests:
 *
 * @code{.sh}
 * export CUTELYST_MEMCACHED_TEST_SERVERS=localhost;/tmp/memcached.sock
 * @endcode
 *
 * @since Cutelyst 1.11.0
 */
class CUTELYST_PLUGIN_MEMCACHED_EXPORT Memcached : public Plugin
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Memcached)
public:
    /**
     * Constructs a new Memcached object with the given @a parent.
     */
    Memcached(Application *parent);

    /**
     * Deconstructs the Memcached object.
     */
    virtual ~Memcached() override;

    /**
     * Return types for memcached operations.
     */
    enum MemcachedReturnType {
        Success,                 /**< The request was successfully executed. */
        Failure,                 /**< A unknown failure has occurred in the server. */
        HostLookupFailure,       /**< A DNS failure has occurred. */
        ConnectionFailure,       /**< A unknown error has occurred while trying to connect to a server. */
        WriteFailure,            /**< An error has occurred while trying to write to a server. */
        ReadFailure,             /**< A read failure has occurred. */
        UnknownReadFailure,      /**< An unknown read failure only occurs when either there is a bug in the server, or in rare cases where an ethernet nic is reporting dubious information. */
        ProtocolError,           /**< An unknown error has occurred in the protocol. */
        ClientError,             /**< An unknown client error has occurred internally. */
        ServerError,             /**< An unknown error has occurred in the server. */
        Error,                   /**< A general error occurred. */
        DataExists,              /**< The data requested with the key given was found. */
        DataDoesNotExist,        /**< The data requested with the key given was not found. */
        NotStored,               /**< The request to store an object failed. */
        Stored,                  /**< The requested object has been successfully stored on the server. */
        NotFound,                /**< The object requested was not found. */
        MemoryAllocationFailure, /**< An error has occurred while trying to allocate memory. */
        PartialRead,             /**< The read was only partially successful. */
        SomeErrors,              /**< A multi request has been made, and some underterminate number of errors have occurred. */
        NoServers,               /**< No servers have been added to the memcached_st object. */
        End,                     /**< The server has completed returning all of the objects requested. */
        Deleted,                 /**< The object requested by the key has been deleted. */
        Stat,                    /**< A “stat” command has been returned in the protocol. */
        Errno,                   /**< An error has occurred in the driver which has set errno. */
        NotSupported,            /**< The given method is not supported in the server. */
        FetchNotFinished,        /**< A request has been made, but the server has not finished the fetch of the last request. */
        Timeout,                 /**< Operation has timed out. */
        Buffered,                /**< The request has been buffered. */
        BadKeyProvided,          /**< The key provided is not a valid key. */
        InvalidHostProtocol,     /**< The server you are connecting too has an invalid protocol. Most likely you are connecting to an older server that does not speak the binary protocol. */
        ServerMarkedDead,        /**< The requested server has been marked dead. */
        UnknownStatKey,          /**< The server you are communicating with has a stat key which has not be defined in the protocol. */
        E2Big,                   /**< Item is too large for the server to store. */
        InvalidArguments,        /**< The arguments supplied to the given function were not valid. */
        KeyTooBig,               /**< The key that has been provided is too large for the given server. */
        AuthProblem,             /**< An unknown issue has occurred during authentication. */
        AuthFailure,             /**< The credentials provided are not valid for this server. */
        AuthContinue,            /**< Authentication has been paused. */
        ParseError,              /**< An error has occurred while trying to parse the configuration string. You should use memparse to determine what the error was. */
        ParseUserError,          /**< An error has occurred in parsing the configuration string. */
        Deprecated,              /**< The method that was requested has been deprecated. */
        InProgress,
        ServerTemporaryDisabled,
        ServerMemoryAllocationFailure,
        MaximumReturn,
        PluginNotRegisterd /**< The Cutelyst Memcached plugin has not been registered to the application. */
    };
    Q_ENUM(MemcachedReturnType)

    /**
     * Sets default configuration values for configuration keys that are not set in
     * the Cutelyst configuratoin file.
     */
    void setDefaultConfig(const QVariantMap &defaultConfig);

    /**
     * Writes the @a value to the memcached server using @a key. If the @a key
     * already exists it will overwrite what is on the server. If the object
     * does not exist it will be written.
     *
     * @param[in] key key of object whose value to set
     * @param[in] value value of object to write to server
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    static bool set(const QString &key, const QByteArray &value, time_t expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Writes the @a value of type @a T to the memcached server using @a key. If
     * the @a key already exists it will overwrite what is on the server. If the
     * object does not exist it will be written.
     *
     * Type @a T has to be serializable into a QByteArray using QDataStream.
     *
     * @param[in] key key of object whose value to set
     * @param[in] value value of type @a T of object to write to server
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    template <typename T>
    static bool set(const QString &key, const T &value, time_t expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Writes the @a value to the memcached server using @a key. If the @a key already
     * exists it will overwrite what is on the server. If the object does not exist it
     * will be written. This method is functionally equivalent to Memcached::set(),
     * except that the free-form @a groupKey can be used to map the @a key to a specific
     * server. This allows related items to be grouped together on a single server for
     * efficiency.
     *
     * @param[in] groupKey key that specifies the server to write to
     * @param[in] key key of object whose value to set
     * @param[in] value value of object to write to server
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    static bool setByKey(const QString &groupKey, const QString &key, const QByteArray &value, time_t expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Writes the @a value of type @a T to the memcached server using @a key. If the @a key
     * already exists it will overwrite what is on the server. If the object does not exist
     * it will be written. This method is functionally equivalent to Memcached::set(), except
     * that the free-form @a groupKey can be used to map the @a key to a specific server.
     * This allows related items to be grouped together on a single server for efficiency.
     *
     * Type @a T has to be serializable into a QByteArray using QDataStream.
     *
     * @param[in] groupKey key that specifies the server to write to
     * @param[in] key key of object whose value to set
     * @param[in] value value of type @a T of object to write to server
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    template <typename T>
    static bool setByKey(const QString &groupKey, const QString &key, const T &value, time_t expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Adds the @a value to the memcached server using @a key. If the object is found on the server
     * an error occurs and this method returns @c false, otherwise the value is stored.
     *
     * As this method returns @c false if the @a key has already been set on the server, you can
     * use the value of the @a returnType to determine the reason. Other than with other errors,
     * failing because of already existing @a key will not be logged.
     *
     * @param[in] key key of object whose value to add
     * @param[in] value value of object to add to server
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    static bool add(const QString &key, const QByteArray &value, time_t expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Adds the @a value of type @a T to the memcached server using @a key. If the object is found
     * on the server an error occurs and this method returns @c false, otherwise the value is stored.
     *
     * Type @a T has to be serializable into a QByteArray using QDataStream.
     *
     * As this method returns @c false if the @a key has already been set on the server, you can use
     * the value of the @a returnType to determine the reason. Other than with other errors, failing
     * because of already existing @a key will not be logged.
     *
     * @param[in] key key of object whose value to add
     * @param[in] value value of type @a T of object to add to server
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[out] returnType optional pointer to MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    template <typename T>
    static bool add(const QString &key, const T &value, time_t expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Adds the @a value to the memcached server using @a key. If the object is found on the server an
     * error occurs and this method returns @c false, otherwise the value is stored. This method is
     * functionally equivalent to Memcached::add(), except that the free-form @a groupKey can be used
     * to map the @a key to a specific server. This allows related items to be grouped together on a
     * single server for efficiency.
     *
     * As this method returns @c false if the @a key has already been set on the server, you can use
     * the value of the @a returnType to determine the reason. Other than with other errors, failing
     * because of already existing @a key will not be logged.
     *
     * @param[in] groupKey key that specifies the server to write to
     * @param[in] key key of object whose value to add
     * @param[in] value value of object to add to server
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    static bool addByKey(const QString &groupKey, const QString &key, const QByteArray &value, time_t expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Adds the @a value of type @a T to the memcached server using @a key. If the object is found on
     * the server an error occurs and this method returns @c false, otherwise the value is stored. This
     * method is functionally equivalent to Memcached::add(), except that the free-form @a groupKey can
     * be used to map the @a key to a specific server. This allows related items to be grouped together
     * on a single server for efficiency.
     *
     * Type @a T has to be serializable into a QByteArray using QDataStream.
     *
     * As this method returns @c false if the @a key has already been set on the server, you can use
     * the value of the @a returnType to determine the reason. Other than with other errors, failing
     * because of already existing @a key will not be logged.
     *
     * @param[in] groupKey key that specifies the server to write to
     * @param[in] key key of object whose value to add
     * @param[in] value value of object to add to server
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    template <typename T>
    static bool addByKey(const QString &groupKey, const QString &key, const T &value, time_t expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Replaces the data of @a key on the server with @a value. If the @a key ist not found on the
     * server an error occures and @c false will be returned.
     *
     * As this method returns @c false if the @a key can not be found on the server, you can use
     * the value of the @a returnType to determine the reason. Other than with other errors, failing
     * because of not found @a key will not be logged.
     *
     * @param[in] key key of object whose value to replace
     * @param[in] value value to replace object on server with
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    static bool replace(const QString &key, const QByteArray &value, time_t expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Replaces the data of @a key on the server with @a value of type @a T. If the @a key ist not
     * found on the server an error occures and @c false will be returned.
     *
     * Type @a T has to be serializable into a QByteArray using QDataStream.
     *
     * As this method returns @c false if the @a key can not be found on the server, you can use
     * the value of the @a returnType to determine the reason. Other than with other errors, failing
     * because of not found @a key will not be logged.
     *
     * @param[in] key key of object whose value to replace
     * @param[in] value value to replace object on server with
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    template <typename T>
    static bool replace(const QString &key, const T &value, time_t expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Replaces the data of @a key on the server with @a value. If the @a key ist not found on the
     * server an error occures and @c false will be returned. This method is functionally equivalent
     * to Memcached::replace(), except that the free-form @a groupKey can be used to map the @a key
     * to a specific server. This allows related items to be grouped together on a single server for
     * efficiency.
     *
     * As this method returns @c false if the @a key can not be found on the server, you can use
     * the value of the @a returnType to determine the reason. Other than with other errors, failing
     * because of not found @a key will not be logged.
     *
     * @param[in] groupKey key that specifies the server to write to
     * @param[in] key key of object whose value to replace
     * @param[in] value value to replace object on server with
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    static bool replaceByKey(const QString &groupKey, const QString &key, const QByteArray &value, time_t expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Replaces the data of @a key on the server with @a value of Type @a T. If the @a key ist not found
     * on the server an error occures and @c false will be returned. This method is functionally equivalent
     * to Memcached::replace(), except that the free-form @a groupKey can be used to map the @a key
     * to a specific server. This allows related items to be grouped together on a single server for
     * efficiency.
     *
     * Type @a T has to be serializable into a QByteArray using QDataStream.
     *
     * As this method returns @c false if the @a key can not be found on the server, you can use
     * the value of the @a returnType to determine the reason. Other than with other errors, failing
     * because of not found @a key will not be logged.
     *
     * @param[in] groupKey key that specifies the server to write to
     * @param[in] key key of object whose value to replace
     * @param[in] value value to replace object on server with
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    template <typename T>
    static bool replaceByKey(const QString &groupKey, const QString &key, const T &value, time_t expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Fetch an individial value from the server identified by @a key. The returned QByteArray will
     * contain the data fetched from the server. If an error occurred or if the @a key could not
     * be found, the returned QByteArray will be @c null. Use QByteArray::isNull() to check for this.
     *
     * As this method returns a @c null byte array if an error occurred as well if the @a key could
     * not be found, you can use the value of the @a returnType to determine the reason. Other than with
     * other errors, failing because of not found @a key will not be logged.
     *
     * @param[in] key key of object whose value to fecth
     * @param[out] cas optional pointer to a variable that takes the CAS value
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return QByteArray containing the data fetched from the server; if an error occurred or the @a key has not been found, this will be @c null.
     */
    static QByteArray get(const QString &key, uint64_t *cas = nullptr, MemcachedReturnType *returnType = nullptr);

    /**
     * Fetch an individial value of type @a T from the server identified by @a key. The returned type @a T
     * will contain the data fetched from the server. If an error occurred or if the @a key could not
     * be found, the returned type @a T will be default constructed.
     *
     * Type @a T has to be deserializable from a QByteArray using QDataStream.
     *
     * As this method returns a default constructed type @a T if an error occurred as well if the @a key
     * could not be found, you can use the value of the @a returnType to determine the reason.
     * Other than with other errors, failing because of not found @a key will not be logged.
     *
     * @par Usage example
     * @code{.cpp}
     * void MyController::index(Context *c)
     * {
     *     //...
     *
     *     QVariantList list = Memcached::get<QVariantList>(QStringLiteral("MyKey"));
     *
     *     //...
     * }
     * @endcode
     *
     * @param[in] key key of object whose value to fecth
     * @param[out] cas optional pointer to a quint32 variable that takes the CAS value
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return type T containing the data fetched from the server; if an error occurred or the @a key has not been found, this will be a default constructed value
     */
    template <typename T>
    static T get(const QString &key, uint64_t *cas = nullptr, MemcachedReturnType *returnType = nullptr);

    /**
     * Fetch an individial value from the server identified by @a key. The returned QByteArray will
     * contain the data fetched from the server. If an error occurred or if the @a key could not
     * be found, the returned QByteArray will be @c null. Use QByteArray::isNull() to check for this.
     * This method behaves in a similar nature as Memcached::get(). The difference is that it takes
     * a @a groupKey that is used for determining which server an object was stored if key partitioning
     * was used for storage.
     *
     * As this method returns a @c null byte array if an error occurred as well if the @a key could
     * not be found, you can use the value of the @a returnType to determine the reason. Other than with
     * other errors, failing because of not found @a key will not be logged.
     *
     * @param[in] groupKey key that specifies the server to fetch from
     * @param[in] key key of object whose value to fecth
     * @param[out] cas optional pointer to a variable that takes the CAS value
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return QByteArray containing the data fetched from the server; if an error occurred or the @a key has not been found, this will be @c null.
     */
    static QByteArray getByKey(const QString &groupKey, const QString &key, uint64_t *cas = nullptr, MemcachedReturnType *returnType = nullptr);

    /**
     * Fetch an individial value from the server identified by @a key. The returned type @a T will
     * contain the data fetched from the server. If an error occurred or if the @a key could not
     * be found, the returned type @a T will be default constructed.
     * This method behaves in a similar nature as Memcached::get(). The difference is that it takes
     * a @a groupKey that is used for determining which server an object was stored if key partitioning
     * was used for storage.
     *
     * Type @a T has to be deserializable from a QByteArray using QDataStream.
     *
     * As this method returns a default constructed type @a T if an error occurred as well if the @a key
     * could not be found, you can use the value of the @a returnType to determine the reason.
     * Other than with other errors, failing because of not found @a key will not be logged.
     *
     * @par Usage example
     * @code{.cpp}
     * void MyController::index(Context *c)
     * {
     *     //...
     *
     *     QVariantList list = Memcached::getByKey<QVariantList>(QStringLiteral("MyGroup"), QStringLiteral("MyKey"));
     *
     *     //...
     * }
     * @endcode
     *
     * @param[in] groupKey key that specifies the server to fetch from
     * @param[in] key key of object whose value to fecth
     * @param[out] cas optional pointer to a variable that takes the CAS value
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return type T containing the data fetched from the server; if an error occurred or the @a key has not been found, this will be a default constructed value
     */
    template <typename T>
    static T getByKey(const QString &groupKey, const QString &key, uint64_t *cas = nullptr, MemcachedReturnType *returnType = nullptr);

    /**
     * Directly deletes a particular @a key.
     *
     * @param[in] key key of object to delete
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    static bool remove(const QString &key, MemcachedReturnType *returnType = nullptr);

    /**
     * Directly deletes a particular @a key in @a groupKey.
     * This method behaves in a similar nature as Memcached::remove(). The difference is that it
     * takes a @a groupKey that is used for determining which server an object was stored if key
     * partitioning was used for storage.
     *
     * @param[in] groupKey key that specifies the server to delete from
     * @param[in] key key of object to delete
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    static bool removeByKey(const QString &groupKey, const QString &key, MemcachedReturnType *returnType = nullptr);

    /**
     * Checks if the @a key exists.
     * @param[in] key key to check
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true if the key exists; @c false otherwise
     */
    static bool exist(const QString &key, MemcachedReturnType *returnType = nullptr);

    /**
     * Checks if the @a key exists in @a groupKey. This method behaves in a similar nature as
     * Memcached::exist(). The difference is that it takes a @a groupKey that is used for
     * determining which server an object was stored if key partitioning was used for storage.
     *
     * @param groupKey key that specifies the server to check on
     * @param key key to check
     * @param returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true if the key exists; @c false otherwise
     */
    static bool existByKey(const QString &groupKey, const QString &key, MemcachedReturnType *returnType = nullptr);

    /**
     * Expiration time constant that can be used in the increment/decrement with initial methods.
     * @    sa incrementWithInitial() incrementWithInitialByKey() decrementWithInitial() decrementWithInitialByKey()
     */
    static const time_t expirationNotAdd;

    /**
     * Increments the value of @a key by @a offset. If there is a valid pointer to @a value, the incremented value
     * will be returned to it.
     *
     * @note Be aware that the memcached server does not detect overflow and underflow.
     *
     * @note This function does not work if encryption is enabled.
     *
     * @param[in] key key to increment
     * @param[in] offset offset for increment
     * @param[out] value optional pointer to a variable that takes the incremented value
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    static bool increment(const QString &key, uint32_t offset, uint64_t *value = nullptr, MemcachedReturnType *returnType = nullptr);

    /**
     * Increments the value of @a key in @a groupKey by @a offset. If there is a valid pointer to @a value, the
     * incremented value will be returned to it. This method behaves in a similar nature as
     * Memcached::increment(). The difference is that it takes a @a groupKey that is used for
     * determining which server an object was stored if key partitioning was used for storage.
     *
     * @note Be aware that the memcached server does not detect overflow and underflow.
     *
     * @note This function does not work if encryption is enabled.
     *
     * @param[in] groupKey key that specifies the server to increment the key on
     * @param[in] key key to increment
     * @param[in] offset offset for increment
     * @param[out] value optional pointer to a variable that takes the incremented value
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    static bool incrementByKey(const QString &groupKey, const QString &key, uint64_t offset, uint64_t *value = nullptr, MemcachedReturnType *returnType = nullptr);

    /**
     * Increments the value of @a key by @a offset. If the object specified by @a key does not exist,
     * one of two things will happen: if the @a expiration value is Memcached::expirationNotAdd, the
     * operation will fail. For all other expiration values, the operation will succeed by seeding
     * the value for that @a key with a @a initial value to expire with the provided @a expiration time.
     * The flags will be set to zero. If there is a valid pointer to @a value, the created or incremented
     * value will be returned to it.
     *
     * @note This method will only work when using the binary protocol.
     *
     * @note Be aware that the memcached server does not detect overflow and underflow.
     *
     * @note This function does not work if encryption is enabled.
     *
     * @param[in] key key to increment or initialize
     * @param[in] offset offset for increment
     * @param[in] initial initial value if key does not exist
     * @param[in] expiration expiration time in seconds
     * @param[out] value optional pointer to a variable that takes the incremented or initialized value
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    static bool incrementWithInitial(const QString &key, uint64_t offset, uint64_t initial, time_t expiration, uint64_t *value = nullptr, MemcachedReturnType *returnType = nullptr);

    /**
     * Increments the value of @a key in @a groupKey by @a offset. If the object specified by @a key does not exist,
     * one of two things will happen: if the expiration value is Memcached::expirationNotAdd, the
     * operation will fail. For all other expiration values, the operation will succeed by seeding
     * the value for that @a key with a @a initial value to expire with the provided expiration time.
     * The flags will be set to zero. If there is a valid pointer to @a value, the created or incremented
     * value will be returned to it.
     *
     * This method behaves in a similar nature as Memcached::incrementWithInitial(). The difference is that
     * it takes a @a groupKey that is used for determining which server an object was stored if key
     * partitioning was used for storage.
     *
     * @note This method will only work when using the binary protocol.
     *
     * @note Be aware that the memcached server does not detect overflow and underflow.
     *
     * @note This function does not work if encryption is enabled.
     *
     * @param[in] groupKey key that specifies the server to increment the key on
     * @param[in] key key to increment or initialize
     * @param[in] offset offset for increment
     * @param[in] initial initial value if key does not exist
     * @param[in] expiration expiration time in seconds
     * @param[out] value optional pointer to a variable that takes the incremented or initialized value
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    static bool incrementWithInitialByKey(const QString &groupKey, const QString &key, uint64_t offset, uint64_t initial, time_t expiration, uint64_t *value = nullptr, MemcachedReturnType *returnType = nullptr);

    /**
     * Decrements the value of @a key by @a offset. If there is a valid pointer to @a value, the decremented value
     * will be returned to it.
     *
     * @note Be aware that the memcached server does not detect overflow and underflow.
     *
     * @note This function does not work if encryption is enabled.
     *
     * @param[in] key key to decrement
     * @param[in] offset offset for decrement
     * @param[out] value optional pointer to a variable that takes the decremented value
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    static bool decrement(const QString &key, uint32_t offset, uint64_t *value = nullptr, MemcachedReturnType *returnType = nullptr);

    /**
     * Drecrements the value of @a key in @a groupKey by @a offset. If there is a valid pointer to @a value, the
     * decremented value will be returned to it. This method behaves in a similar nature as
     * Memcached::decrement(). The difference is that it takes a @a groupKey that is used for
     * determining which server an object was stored if key partitioning was used for storage.
     *
     * @note Be aware that the memcached server does not detect overflow and underflow.
     *
     * @note This function does not work if encryption is enabled.
     *
     * @param[in] groupKey key that specifies the server to decrement the key on
     * @param[in] key key to decrement
     * @param[in] offset offset for decrement
     * @param[out] value optional pointer to a variable that takes the decremented value
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    static bool decrementByKey(const QString &groupKey, const QString &key, uint64_t offset, uint64_t *value = nullptr, MemcachedReturnType *returnType = nullptr);

    /**
     * Decrements the value of @a key by @a offset. If the object specified by @a key does not exist,
     * one of two things will happen: if the @a expiration value is Memcached::expirationNotAdd, the
     * operation will fail. For all other expiration values, the operation will succeed by seeding
     * the value for that @a key with a @a initial value to expire with the provided @a expiration time.
     * The flags will be set to zero. If there is a valid pointer to @a value, the created or decremented
     * value will be returned to it.
     *
     * @note This method will only work when using the binary protocol.
     *
     * @note Be aware that the memcached server does not detect overflow and underflow.
     *
     * @note This function does not work if encryption is enabled.
     *
     * @param[in] key key to decrement or initialize
     * @param[in] offset offset for decrement
     * @param[in] initial initial value if key does not exist
     * @param[in] expiration expiration time in seconds
     * @param[out] value optional pointer to a variable that takes the decremented or initialized value
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    static bool decrementWithInitial(const QString &key, uint64_t offset, uint64_t initial, time_t expiration, uint64_t *value = nullptr, MemcachedReturnType *returnType = nullptr);

    /**
     * Decrements the value of @a key in @a groupKey by @a offset. If the object specified by @a key does not exist,
     * one of two things will happen: if the @a expiration value is Memcached::expirationNotAdd, the
     * operation will fail. For all other expiration values, the operation will succeed by seeding
     * the value for that @a key with a @a initial value to expire with the provided @a expiration time.
     * The flags will be set to zero. If there is a valid pointer to @a value, the created or decremented
     * value will be returned to it.
     *
     * This method behaves in a similar nature as Memcached::decrementWithInitial(). The difference is that
     * it takes a @a groupKey that is used for determining which server an object was stored if key
     * partitioning was used for storage.
     *
     * @note This method will only work when using the binary protocol.
     *
     * @note Be aware that the memcached server does not detect overflow and underflow.
     *
     * @note This function does not work if encryption is enabled.
     *
     * @param[in] groupKey key that specifies the server to decrement or initialize the key on
     * @param[in] key key to decrement or initialize
     * @param[in] offset offset for decrement
     * @param[in] initial initial value if key does not exist
     * @param[in] expiration expiration time in seconds
     * @param[out] value optional pointer to a variable that takes the decremented or initialized value
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    static bool decrementWithInitialByKey(const QString &groupKey, const QString &key, uint64_t offset, uint64_t initial, time_t expiration, uint64_t *value = nullptr, MemcachedReturnType *returnType = nullptr);

    /**
     * Overwrites data for @a key in the server as long as the @a cas value is still the same in the server.
     * You can get the @a cas value by using the cas return value of Memcached::get()
     *
     * @param[in] key key of object whose value to compare and set
     * @param[in] value value of object to write to server
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[in] cas the cas value to compare
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    static bool cas(const QString &key, const QByteArray &value, time_t expiration, uint64_t cas, MemcachedReturnType *returnType = nullptr);

    /**
     * Overwrites data for @a key in the server as long as the @a cas value is still the same in the server.
     * You can get the @a cas value by using the cas return value of Memcached::get()
     *
     * Type @a T has to be serializable into a QByteArray using QDataStream.
     *
     * @param[in] key key of object whose value to compare and set
     * @param[in] value value of type @a T of object to write to server
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[in] cas the cas value to compare
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    template <typename T>
    static bool cas(const QString &key, const T &value, time_t expiration, uint64_t cas, MemcachedReturnType *returnType = nullptr);

    /**
     * Overwrites data for @a key in @a groupKey in the server as long as the @a cas value is still the same in the server.
     * You can get the @a cas value by using the cas return value of Memcached::getByKey().
     *
     * This method behaves in a similar nature as Memcached::cas(). The difference is that
     * it takes a @a groupKey that is used for determining which server an object was stored if key
     * partitioning was used for storage.
     *
     * @param[in] groupkey key that specifies the server to write to
     * @param[in] key key of object whose value to compare and set
     * @param[in] value value of object to write to server
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[in] cas the cas value to compare
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    static bool casByKey(const QString &groupKey, const QString &key, const QByteArray &value, time_t expiration, uint64_t cas, MemcachedReturnType *returnType = nullptr);

    /**
     * Overwrites data for @a key in @a groupKey in the server as long as the @a cas value is still the same in the server.
     * You can get the @a cas value by using the cas return value of Memcached::getByKey()
     *
     * Type @a T has to be serializable into a QByteArray using QDataStream.
     *
     * This method behaves in a similar nature as Memcached::cas(). The difference is that
     * it takes a @a groupKey that is used for determining which server an object was stored if key
     * partitioning was used for storage.
     *
     * @param[in] groupkey key that specifies the server to write to
     * @param[in] key key of object whose value to compare and set
     * @param[in] value value of type @a T of object to write to server
     * @param[in] expiration time in seconds to keep the object stored in the server
     * @param[in] cas the cas value to compare
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    template <typename T>
    static bool casByKey(const QString &groupKey, const QString &key, const T &value, time_t expiration, uint64_t cas, MemcachedReturnType *returnType = nullptr);

    /**
     * Used in conjunction with buffer requests enabled to flush all buffers by sending the buffered commands to the server
     * for processing.
     *
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    static bool flushBuffers(MemcachedReturnType *returnType = nullptr);

    /**
     * Wipe cleans the contents of memcached servers. It will either do this immediately or expire the content
     * based on the @a expiration time passed to the method (a value of zero causes an immediate flush). The
     * operation is not atomic to multiple servers, just atomic to a single server. That is, it will flush
     * the servers in the order that they were added.
     *
     * @param[in] expiration time in seconds
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    static bool flush(time_t expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Fetch multiple values from the server identified by a list of @a keys. If a pointer for the @a casValues
     * is provided, keys and their cas values will be written to it.
     *
     * As this might return an empty QHash if nothing has been found or if an error occurred, you can
     * use the @a returnType pointer to determine the reason.
     *
     * @param[in] keys list of keys to fetch from the server
     * @param[out] casValues optional pointer to a QHash that will contain keys and their cas values
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return QHash containing the keys and values
     */
    static QHash<QString, QByteArray> mget(const QStringList &keys, QHash<QString, uint64_t> *casValues = nullptr, MemcachedReturnType *returnType = nullptr);

    /**
     * Fetch multiple values of type @a T from the server identified by a list of @a keys. If a pointer
     * for the @a casValues is provided, keys and their cas values are written to it.
     *
     * As this might return an empty QHash if nothing has been found or if an error occurred, you can
     * use the @a returnType pointer to determine the reason.
     *
     * Type @a T has to be deserializable from a QByteArray using QDataStream.
     *
     * @param[in] keys list of keys to fetch from the server
     * @param[out] casValues optional pointer to a QHash that will contain keys and their cas values
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return QHash containing the keys and values
     */
    template <typename T>
    static QHash<QString, T> mget(const QStringList &keys, QHash<QString, uint64_t> *casValues = nullptr, MemcachedReturnType *returnType = nullptr);

    /**
     * Fetch multiple values from the server specified by @a groupKey identified by a list of @a keys.
     * If a pointer for the @a casValues is provided, keys and their cas values will be written to it.
     *
     * As this might return an empty QHash if nothing has been found or if an error occurred, you can
     * use the @a returnType pointer to determine the reason.
     *
     * This method behaves in a similar nature as Memcached::mget(). The difference is that
     * it takes a @a groupKey that is used for determining which server an object was stored if key
     * partitioning was used for storage.
     *
     * @param[in] groupKey key to specify the server to fetch values from
     * @param[in] keys list of keys to fetch from the server
     * @param[out] casValues optional pointer to a QHash that will contain keys and their cas values
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return QHash containing the keys and values
     */
    static QHash<QString, QByteArray> mgetByKey(const QString &groupKey, const QStringList &keys, QHash<QString, uint64_t> *casValues = nullptr, MemcachedReturnType *returnType = nullptr);

    /**
     * Fetch multiple values of type @a T from the server specified by @a groupKey identified by a list of @a keys.
     * If a pointer for the @a casValues is provided, keys and their cas values will be written to it.
     *
     * As this might return an empty QHash if nothing has been found or if an error occurred, you can
     * use the @a returnType pointer to determine the reason.
     *
     * Type @a T has to be deserializable from a QByteArray using QDataStream.
     *
     * This method behaves in a similar nature as Memcached::mget(). The difference is that
     * it takes a @a groupKey that is used for determining which server an object was stored if key
     * partitioning was used for storage.
     *
     * @param[in] groupKey key to specify the server to fetch values from
     * @param[in] keys list of keys to fetch from the server
     * @param[out] casValues optional pointer to a QHash that will contain keys and their cas values
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return QHash containing the keys and values
     */
    template <typename T>
    static QHash<QString, T> mgetByKey(const QString &groupKey, const QStringList &keys, QHash<QString, uint64_t> *casValues = nullptr, MemcachedReturnType *returnType = nullptr);

    /**
     * Updates the @a expiration time on an existing @a key.
     *
     * @param[in] key key whose expiration time to update
     * @param[in] expiration new expiration time in seconds
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    static bool touch(const QString &key, time_t expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Updates the @a expiration time on an existing @a key in group @a groupKey.
     *
     * This method behaves in a similar nature as Memcached::touch(). The difference is that
     * it takes a @a groupKey that is used for determining which server an object was stored if key
     * partitioning was used for storage.
     *
     * @param[in] groupKey key to specify the server to update the key on
     * @param[in] key key whose expiration time to update
     * @param[in] expiration new expiration time in seconds
     * @param[out] returnType optional pointer to a MemcachedReturnType variable that takes the return type of the operation
     * @return @c true on success; @c false otherwise
     */
    static bool touchByKey(const QString &groupKey, const QString &key, time_t expiration, MemcachedReturnType *returnType = nullptr);

    /**
     * Converts the return type @a rt into human readable error string.
     */
    static QString errorString(Context *c, MemcachedReturnType rt);

    /*!
     * Returns the version of the currently used libmemcached.
     */
    static QVersionNumber libMemcachedVersion();

protected:
    const QScopedPointer<MemcachedPrivate> d_ptr;

    /**
     * Reads the configuration and sets up the plugin.
     */
    virtual bool setup(Application *app) override;
};

template <typename T>
bool Memcached::set(const QString &key, const T &value, time_t expiration, MemcachedReturnType *returnType)
{
    QByteArray data;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QDataStream out(&data, QIODevice::WriteOnly);
#else
    QDataStream out(&data, QIODeviceBase::WriteOnly);
#endif
    out << value;
    return Memcached::set(key, data, expiration, returnType);
}

template <typename T>
bool Memcached::setByKey(const QString &groupKey, const QString &key, const T &value, time_t expiration, MemcachedReturnType *returnType)
{
    QByteArray data;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QDataStream out(&data, QIODevice::WriteOnly);
#else
    QDataStream out(&data, QIODeviceBase::WriteOnly);
#endif
    out << value;
    return Memcached::setByKey(groupKey, key, data, expiration, returnType);
}

template <typename T>
bool Memcached::add(const QString &key, const T &value, time_t expiration, MemcachedReturnType *returnType)
{
    QByteArray data;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QDataStream out(&data, QIODevice::WriteOnly);
#else
    QDataStream out(&data, QIODeviceBase::WriteOnly);
#endif
    out << value;
    return Memcached::add(key, data, expiration, returnType);
}

template <typename T>
bool Memcached::addByKey(const QString &groupKey, const QString &key, const T &value, time_t expiration, MemcachedReturnType *returnType)
{
    QByteArray data;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QDataStream out(&data, QIODevice::WriteOnly);
#else
    QDataStream out(&data, QIODeviceBase::WriteOnly);
#endif
    out << value;
    return Memcached::addByKey(groupKey, key, data, expiration, returnType);
}

template <typename T>
bool Memcached::replace(const QString &key, const T &value, time_t expiration, MemcachedReturnType *returnType)
{
    QByteArray data;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QDataStream out(&data, QIODevice::WriteOnly);
#else
    QDataStream out(&data, QIODeviceBase::WriteOnly);
#endif
    out << value;
    return Memcached::replace(key, data, expiration, returnType);
}

template <typename T>
bool Memcached::replaceByKey(const QString &groupKey, const QString &key, const T &value, time_t expiration, MemcachedReturnType *returnType)
{
    QByteArray data;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QDataStream out(&data, QIODevice::WriteOnly);
#else
    QDataStream out(&data, QIODeviceBase::WriteOnly);
#endif
    out << value;
    return Memcached::replaceByKey(groupKey, key, data, expiration, returnType);
}

template <typename T>
T Memcached::get(const QString &key, uint64_t *cas, MemcachedReturnType *returnType)
{
    T retVal;
    QByteArray ba = Memcached::get(key, cas, returnType);
    if (!ba.isEmpty()) {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        QDataStream in(&ba, QIODevice::ReadOnly);
#else
        QDataStream in(&ba, QIODeviceBase::ReadOnly);
#endif
        in >> retVal;
    }
    return retVal;
}

template <typename T>
T Memcached::getByKey(const QString &groupKey, const QString &key, uint64_t *cas, MemcachedReturnType *returnType)
{
    T retVal;
    QByteArray ba = Memcached::getByKey(groupKey, key, cas, returnType);
    if (!ba.isEmpty()) {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        QDataStream in(&ba, QIODevice::ReadOnly);
#else
        QDataStream in(&ba, QIODeviceBase::ReadOnly);
#endif
        in >> retVal;
    }
    return retVal;
}

template <typename T>
bool Memcached::cas(const QString &key, const T &value, time_t expiration, uint64_t cas, MemcachedReturnType *returnType)
{
    QByteArray data;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QDataStream out(&data, QIODevice::WriteOnly);
#else
    QDataStream out(&data, QIODeviceBase::WriteOnly);
#endif
    out << value;
    return Memcached::cas(key, data, expiration, cas, returnType);
}

template <typename T>
bool Memcached::casByKey(const QString &groupKey, const QString &key, const T &value, time_t expiration, uint64_t cas, MemcachedReturnType *returnType)
{
    QByteArray data;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QDataStream out(&data, QIODevice::WriteOnly);
#else
    QDataStream out(&data, QIODeviceBase::WriteOnly);
#endif
    out << value;
    return Memcached::casByKey(groupKey, key, data, expiration, cas, returnType);
}

template <typename T>
QHash<QString, T> Memcached::mget(const QStringList &keys, QHash<QString, uint64_t> *casValues, MemcachedReturnType *returnType)
{
    QHash<QString, T> hash;
    QHash<QString, QByteArray> _data = Memcached::mget(keys, casValues, returnType);
    if (!_data.empty()) {
        auto i = _data.constBegin();
        while (i != _data.constEnd()) {
            T retVal;
            QDataStream in(i.value());
            in >> retVal;
            hash.insert(i.key(), retVal);
            ++i;
        }
    }
    return hash;
}

template <typename T>
QHash<QString, T> Memcached::mgetByKey(const QString &groupKey, const QStringList &keys, QHash<QString, uint64_t> *casValues, MemcachedReturnType *returnType)
{
    QHash<QString, T> hash;
    QHash<QString, QByteArray> _data = Memcached::mgetByKey(groupKey, keys, casValues, returnType);
    if (!_data.empty()) {
        auto i = _data.constBegin();
        while (i != _data.constEnd()) {
            T retVal;
            QDataStream in(i.value());
            in >> retVal;
            hash.insert(i.key(), retVal);
            ++i;
        }
    }
    return hash;
}

} // namespace Cutelyst

#endif // CUTELYSTMEMCACHED_H
