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

#include "memcached_p.h"

#include <Cutelyst/Application>
#include <Cutelyst/Engine>
#include <Cutelyst/Context>

#include <utility>
#include <QStringList>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(C_MEMCACHED, "cutelyst.plugin.memcached")

using namespace Cutelyst;

static thread_local Memcached *mcd = nullptr;
const time_t Memcached::expirationNotAdd = MEMCACHED_EXPIRATION_NOT_ADD;

Memcached::Memcached(Application *parent) :
    Plugin(parent), d_ptr(new MemcachedPrivate)
{

}

Memcached::~Memcached()
{

}

void Memcached::setDefaultConfig(const QVariantMap &defaultConfig)
{
    Q_D(Memcached);
    d->defaultConfig = defaultConfig;
}

bool Memcached::setup(Application *app)
{
    Q_D(Memcached);

    const QVariantMap map = app->engine()->config(QStringLiteral("Cutelyst_Memcached_Plugin"));
    QStringList config;

    const QStringList serverList = map.value(QStringLiteral("servers")).toString().split(QLatin1Char(';'));
    if (!serverList.empty()) {
        for (const QString &server : serverList) {
            const QStringList serverParts = server.split(QLatin1Char(','));
            QString name;
            QString port = QStringLiteral("11211");
            QString weight = QStringLiteral("1");
            if (!serverParts.empty()) {
                const QString _name = serverParts.at(0);
                if (!_name.isEmpty()) {
                    name = _name;
                }
                if (serverParts.size() > 1) {
                    const QString _port = serverParts.at(1);
                    if (!_port.isEmpty()) {
                        port = _port;
                    }
                    if (serverParts.size() > 2) {
                        const QString _weight = serverParts.at(2);
                        if (!_weight.isEmpty()) {
                            weight = _weight;
                        }
                    }
                }
            }
            if (!name.isEmpty()) {
                if (name.startsWith(QLatin1Char('/'))) {
                    config.push_back(QLatin1String("--SOCKET=") + name + QLatin1String("/?") + weight);
                } else {
                    config.push_back(QLatin1String("--SERVER=") + name + QLatin1Char(':') + port + QLatin1String("/?") + weight);
                }
            }
        }
    }

    if (config.empty()) {
        config.push_back(QStringLiteral("--SERVER=localhost"));
    }

    for (const QString &flag : {
         QStringLiteral("verify_key"),
         QStringLiteral("remove_failed_servers"),
         QStringLiteral("binary_protocol"),
         QStringLiteral("buffer_requests"),
         QStringLiteral("hash_with_namespace"),
         QStringLiteral("noreply"),
         QStringLiteral("randomize_replica_read"),
         QStringLiteral("sort_hosts"),
         QStringLiteral("support_cas"),
         QStringLiteral("use_udp"),
         QStringLiteral("tcp_nodelay"),
         QStringLiteral("tcp_keepalive")
    }) {
        if (map.value(flag, d->defaultConfig.value(flag, false).toBool()).toBool()) {
            const QString flagStr = QLatin1String("--") + flag.toUpper().replace(QLatin1Char('_'), QLatin1Char('-'));
            config.push_back(flagStr);
        }
    }

    for (const QString &opt : {
         QStringLiteral("connect_timeout"),
         QStringLiteral("distribution"),
         QStringLiteral("hash"),
         QStringLiteral("number_of_replicas"),
         QStringLiteral("namespace"),
         QStringLiteral("retry_timeout"),
         QStringLiteral("server_failure_limit"),
         QStringLiteral("snd_timeout"),
         QStringLiteral("socket_recv_size"),
         QStringLiteral("socket_send_size"),
         QStringLiteral("poll_timeout"),
         QStringLiteral("io_bytes_watermark"),
         QStringLiteral("io_key_prefetch"),
         QStringLiteral("io_msg_watermark"),
         QStringLiteral("rcv_timeout")
    }) {
        QString _val = map.value(opt, d->defaultConfig.value(opt).toString()).toString();
        if (!_val.isEmpty()) {
            const QString optStr = QLatin1String("--") + opt.toUpper().replace(QLatin1Char('_'), QLatin1Char('-')) + QLatin1Char('=') + _val;
            config.push_back(optStr);
        }
    }

    const QByteArray configString = config.join(QChar(QChar::Space)).toUtf8();

    bool ok = false;

    qCInfo(C_MEMCACHED, "Setting up connection to memcached servers using libmemcached %s with the following configuration string: \"%s\"", memcached_lib_version(), configString.constData());

    memcached_st *new_memc = memcached(configString.constData(), configString.size());

    if (new_memc) {
        d->compression = map.value(QStringLiteral("compression"), false).toBool();
        d->compressionLevel = map.value(QStringLiteral("compression_level"), -1).toInt();
        d->compressionThreshold = map.value(QStringLiteral("compression_threshold"), 100).toInt();
        if (d->compression) {
            qCInfo(C_MEMCACHED, "Compression: enabled (Compression level: %i, Compression threshold: %i bytes)", d->compressionLevel, d->compressionThreshold);
        } else {
            qCInfo(C_MEMCACHED, "Compression: disabled");
        }

        const QString encKey = map.value(QStringLiteral("encryption_key")).toString();
        if (!encKey.isEmpty()) {
            const QByteArray encKeyBa = encKey.toUtf8();
            const memcached_return_t rt = memcached_set_encoding_key(new_memc, encKeyBa.constData(), encKeyBa.size());
            if (Q_LIKELY(memcached_success(rt))) {
                qCInfo(C_MEMCACHED, "Encryption: enabled");
            } else {
                qCWarning(C_MEMCACHED, "Failed to enable encryption: %s", memcached_strerror(new_memc, rt));
            }
        } else {
            qCInfo(C_MEMCACHED, "Encryption: disabled");
        }

        const QString saslUser = map.value(QStringLiteral("sasl_user")).toString();
        const QString saslPass = map.value(QStringLiteral("sasl_password")).toString();
        if (!saslUser.isEmpty() && !saslPass.isEmpty()) {
            const memcached_return_t rt = memcached_set_sasl_auth_data(new_memc, saslUser.toUtf8().constData(), saslPass.toUtf8().constData());
            if (Q_LIKELY(memcached_success(rt))) {
                qCInfo(C_MEMCACHED, "SASL authentication: enabled");
                d->saslEnabled = true;
            } else {
                qCWarning(C_MEMCACHED, "Failed to enable SASL authentication: %s", memcached_strerror(new_memc, rt));
            }
        } else {
            qCInfo(C_MEMCACHED, "SASL authentication: disabled");
        }

        if (d->memc) {
            memcached_free(d->memc);
        }
        d->memc = new_memc;
        ok = true;
    }

    if (ok) {
        connect(app, &Application::postForked, this, &MemcachedPrivate::_q_postFork);
    } else {
        qCCritical(C_MEMCACHED) << "Failed to configure the connection to the memcached server(s)";
    }

    return ok;
}

bool Memcached::set(const QString &key, const QByteArray &value, time_t expiration, Cutelyst::Memcached::MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const QByteArray _key = key.toUtf8();

    MemcachedPrivate::Flags flags;
    QByteArray _value = value;

    if (mcd->d_ptr->compression && (_value.size() > mcd->d_ptr->compressionThreshold)) {
        flags |= MemcachedPrivate::Compressed;
        _value = qCompress(value, mcd->d_ptr->compressionLevel);
    }

    const memcached_return_t rt = memcached_set(mcd->d_ptr->memc,
                                                _key.constData(),
                                                _key.size(),
                                                _value.constData(),
                                                _value.size(),
                                                expiration,
                                                flags);

    const bool ok = memcached_success(rt);

    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to store key \"%s\": %s", _key.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::setByKey(const QString &groupKey, const QString &key, const QByteArray &value, time_t expiration, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const QByteArray _key = key.toUtf8();
    const QByteArray _groupKey = groupKey.toUtf8();

    MemcachedPrivate::Flags flags;
    QByteArray _value = value;

    if (mcd->d_ptr->compression && (_value.size() > mcd->d_ptr->compressionThreshold)) {
        flags |= MemcachedPrivate::Compressed;
        _value = qCompress(value, mcd->d_ptr->compressionLevel);
    }

    const memcached_return_t rt = memcached_set_by_key(mcd->d_ptr->memc,
                                                       _groupKey.constData(),
                                                       _groupKey.size(),
                                                       _key.constData(),
                                                       _key.size(),
                                                       _value.constData(),
                                                       _value.size(),
                                                       expiration,
                                                       flags);

    const bool ok = memcached_success(rt);

    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to store key \"%s\" on group \"%s\": %s", _key.constData(), _groupKey.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::add(const QString &key, const QByteArray &value, time_t expiration, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const QByteArray _key = key.toUtf8();

    MemcachedPrivate::Flags flags;
    QByteArray _value = value;

    if (mcd->d_ptr->compression && (_value.size() > mcd->d_ptr->compressionThreshold)) {
        flags |= MemcachedPrivate::Compressed;
        _value = qCompress(value, mcd->d_ptr->compressionLevel);
    }

    const memcached_return_t rt = memcached_add(mcd->d_ptr->memc,
                                                _key.constData(),
                                                _key.size(),
                                                _value.constData(),
                                                _value.size(),
                                                expiration,
                                                flags);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTSTORED)) {
        qCWarning(C_MEMCACHED, "Failed to add key \"%s\": %s", _key.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::addByKey(const QString &groupKey, const QString &key, const QByteArray &value, time_t expiration, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const QByteArray _key = key.toUtf8();
    const QByteArray _groupKey = groupKey.toUtf8();

    MemcachedPrivate::Flags flags;
    QByteArray _value = value;

    if (mcd->d_ptr->compression && (_value.size() > mcd->d_ptr->compressionThreshold)) {
        flags |= MemcachedPrivate::Compressed;
        _value = qCompress(value, mcd->d_ptr->compressionLevel);
    }

    const memcached_return_t rt = memcached_add_by_key(mcd->d_ptr->memc,
                                                      _groupKey.constData(),
                                                      _groupKey.size(),
                                                      _key.constData(),
                                                      _key.size(),
                                                      _value.constData(),
                                                      _value.size(),
                                                      expiration,
                                                      flags);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTSTORED)) {
        qCWarning(C_MEMCACHED, "Failed to add key \"%s\" on group \"%s\": %s", _key.constData(), _groupKey.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::replace(const QString &key, const QByteArray &value, time_t expiration, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const QByteArray _key = key.toUtf8();

    MemcachedPrivate::Flags flags;
    QByteArray _value = value;

    if (mcd->d_ptr->compression && (_value.size() > mcd->d_ptr->compressionThreshold)) {
        flags |= MemcachedPrivate::Compressed;
        _value = qCompress(value, mcd->d_ptr->compressionLevel);
    }

    const memcached_return_t rt = memcached_replace(mcd->d_ptr->memc,
                                                    _key.constData(),
                                                    _key.size(),
                                                    _value.constData(),
                                                    _value.size(),
                                                    expiration,
                                                    flags);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTSTORED)) {
        qCWarning(C_MEMCACHED, "Failed to replace key \"%s\": %s", _key.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::replaceByKey(const QString &groupKey, const QString &key, const QByteArray &value, time_t expiration, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const QByteArray _groupKey = groupKey.toUtf8();
    const QByteArray _key = key.toUtf8();

    MemcachedPrivate::Flags flags;
    QByteArray _value = value;

    if (mcd->d_ptr->compression && (_value.size() > mcd->d_ptr->compressionThreshold)) {
        flags |= MemcachedPrivate::Compressed;
        _value = qCompress(value, mcd->d_ptr->compressionLevel);
    }

    const memcached_return_t rt = memcached_replace_by_key(mcd->d_ptr->memc,
                                                           _groupKey.constData(),
                                                           _groupKey.size(),
                                                           _key.constData(),
                                                           _key.size(),
                                                           _value.constData(),
                                                           _value.size(),
                                                           expiration,
                                                           flags);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTSTORED)) {
        qCWarning(C_MEMCACHED, "Failed to replace key \"%s\" on group \"%s\": %s", _key.constData(), _groupKey.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

QByteArray Memcached::get(const QString &key, uint64_t *cas, Cutelyst::Memcached::MemcachedReturnType *returnType)
{
    QByteArray retData;

    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return retData;
    }

    memcached_return_t rt;
    const QByteArray _key = key.toUtf8();
    bool ok = false;

    std::vector<const char *> keys;
    std::vector<size_t> sizes;
    keys.push_back(_key.constData());
    sizes.push_back(_key.size());
    rt = memcached_mget(mcd->d_ptr->memc,
                        &keys[0],
                        &sizes[0],
                        keys.size());

    if (memcached_success(rt)) {
        memcached_result_st *result = memcached_fetch_result(mcd->d_ptr->memc, NULL, &rt);
        if (result) {
            retData = QByteArray(memcached_result_value(result), memcached_result_length(result));
            if (cas) {
                *cas = memcached_result_cas(result);
            }
            MemcachedPrivate::Flags flags = MemcachedPrivate::Flags(memcached_result_flags(result));
            if (flags.testFlag(MemcachedPrivate::Compressed)) {
                retData = qUncompress(retData);
            }
            ok = true;
            // fetch another result even if there is no one to get
            // a NULL for the internal of libmemcached
            memcached_fetch_result(mcd->d_ptr->memc, NULL, NULL);
        }
        memcached_result_free(result);
    }

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED, "Failed to get data for key \"%s\": %s", _key.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return retData;
}

QByteArray Memcached::getByKey(const QString &groupKey, const QString &key, uint64_t *cas, MemcachedReturnType *returnType)
{
    QByteArray retData;

    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return retData;
    }

    memcached_return_t rt;
    const QByteArray _groupKey = groupKey.toUtf8();
    const QByteArray _key = key.toUtf8();
    bool ok = false;

    std::vector<const char *> keys;
    std::vector<size_t> sizes;
    keys.push_back(_key.constData());
    sizes.push_back(_key.size());
    rt = memcached_mget_by_key(mcd->d_ptr->memc,
                               _groupKey.constData(),
                               _groupKey.size(),
                               &keys[0],
                               &sizes[0],
                               keys.size());

    if (memcached_success(rt)) {
        memcached_result_st *result = memcached_fetch_result(mcd->d_ptr->memc, NULL, &rt);
        if (result) {
            retData = QByteArray(memcached_result_value(result), memcached_result_length(result));
            if (cas) {
                *cas = memcached_result_cas(result);
            }
            MemcachedPrivate::Flags flags = MemcachedPrivate::Flags(memcached_result_flags(result));
            if (flags.testFlag(MemcachedPrivate::Compressed)) {
                retData = qUncompress(retData);
            }
            ok = true;
            // fetch another result even if there is no one to get
            // a NULL for the internal of libmemcached
            memcached_fetch_result(mcd->d_ptr->memc, NULL, NULL);
        }
        memcached_result_free(result);
    }

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED, "Failed to get data for key \"%s\" on group \"%s\": %s", _key.constData(), _groupKey.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return retData;
}

bool Memcached::remove(const QString &key, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const QByteArray _key = key.toUtf8();

    const memcached_return_t rt = memcached_delete(mcd->d_ptr->memc,
                                                   _key.constData(),
                                                   _key.size(),
                                                   0);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED, "Failed to remove data for key \"%s\": %s", _key.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::removeByKey(const QString &groupKey, const QString &key, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const QByteArray _groupKey = groupKey.toUtf8();
    const QByteArray _key = key.toUtf8();

    const memcached_return_t rt = memcached_delete_by_key(mcd->d_ptr->memc,
                                                          _groupKey.constData(),
                                                          _groupKey.size(),
                                                          _key.constData(),
                                                          _key.size(),
                                                          0);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED, "Failed to remove data for key \"%s\" on group \"%s\": %s", _key.constData(), _groupKey.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    return ok;
}

bool Memcached::exist(const QString &key, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const QByteArray _key = key.toUtf8();

    const memcached_return_t rt = memcached_exist(mcd->d_ptr->memc,
                                                  _key.constData(),
                                                  _key.size());

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED, "Failed to check existence of key \"%s\": %s", _key.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::existByKey(const QString &groupKey, const QString &key, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const QByteArray _groupKey = groupKey.toUtf8();
    const QByteArray _key = key.toUtf8();

    const memcached_return_t rt = memcached_exist_by_key(mcd->d_ptr->memc,
                                                         _groupKey.constData(),
                                                         _groupKey.size(),
                                                         _key.constData(),
                                                         _key.size());

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED, "Failed to check existence of key \"%s\" in group \"%s\": %s", _key.constData(), _groupKey.constData());
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::increment(const QString &key, uint32_t offset, uint64_t *value, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const QByteArray _key = key.toUtf8();

    const memcached_return_t rt = memcached_increment(mcd->d_ptr->memc,
                                                      _key.constData(),
                                                      _key.size(),
                                                      offset,
                                                      value);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED, "Failed to increment key \"%s\" by %lu: %s", _key.constData(), offset, memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::incrementByKey(const QString &groupKey, const QString &key, uint64_t offset, uint64_t *value, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const QByteArray _group = groupKey.toUtf8();
    const QByteArray _key = key.toUtf8();

    const memcached_return_t rt = memcached_increment_by_key(mcd->d_ptr->memc,
                                                             _group.constData(),
                                                             _group.size(),
                                                             _key.constData(),
                                                             _key.size(),
                                                             offset,
                                                             value);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED, "Failed to increment \"%s\" key on group \"%s\" by %lu: %s", _key.constData(), _group.constData(), offset, memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::incrementWithInitial(const QString &key, uint64_t offset, uint64_t initial, time_t expiration, uint64_t *value, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const QByteArray _key = key.toUtf8();

    const memcached_return_t rt = memcached_increment_with_initial(mcd->d_ptr->memc,
                                                                   _key.constData(),
                                                                   _key.size(),
                                                                   offset,
                                                                   initial,
                                                                   expiration,
                                                                   value);

    const bool ok = memcached_success(rt);

    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to increment or initialize key \"%s\" by offset %lu or initial %lu: %s", _key.constData(), offset, initial, memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::incrementWithInitialByKey(const QString &groupKey, const QString &key, uint64_t offset, uint64_t initial, time_t expiration, uint64_t *value, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const QByteArray _group = groupKey.toUtf8();
    const QByteArray _key = key.toUtf8();

    const memcached_return_t rt = memcached_increment_with_initial_by_key(mcd->d_ptr->memc,
                                                                          _group.constData(),
                                                                          _group.size(),
                                                                          _key.constData(),
                                                                          _key.size(),
                                                                          offset,
                                                                          initial,
                                                                          expiration,
                                                                          value);

    const bool ok = memcached_success(rt);
    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to increment or initialize key \"%s\" in group \"%s\" by offset %lu or initial %lu: %s", _key.constData(), _group.constData(), offset, initial, memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::decrement(const QString &key, uint32_t offset, uint64_t *value, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const QByteArray _key = key.toUtf8();

    const memcached_return_t rt = memcached_decrement(mcd->d_ptr->memc,
                                                      _key.constData(),
                                                      _key.size(),
                                                      offset,
                                                      value);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED, "Failed to decrement key \"%s\" by %lu: %s", _key.constData(), offset, memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::decrementByKey(const QString &groupKey, const QString &key, uint64_t offset, uint64_t *value, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const QByteArray _group = groupKey.toUtf8();
    const QByteArray _key = key.toUtf8();

    const memcached_return_t rt = memcached_decrement_by_key(mcd->d_ptr->memc,
                                                             _group.constData(),
                                                             _group.size(),
                                                             _key.constData(),
                                                             _key.size(),
                                                             offset,
                                                             value);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED, "Failed to decrement \"%s\" key on group \"%s\" by %lu: %s", _key.constData(), _group.constData(), offset, memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::decrementWithInitial(const QString &key, uint64_t offset, uint64_t initial, time_t expiration, uint64_t *value, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const QByteArray _key = key.toUtf8();

    const memcached_return_t rt = memcached_decrement_with_initial(mcd->d_ptr->memc,
                                                                   _key.constData(),
                                                                   _key.size(),
                                                                   offset,
                                                                   initial,
                                                                   expiration,
                                                                   value);

    const bool ok = memcached_success(rt);

    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to decrement or initialize key \"%s\" by offset %lu or initial %lu: %s", _key.constData(), offset, initial, memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::decrementWithInitialByKey(const QString &groupKey, const QString &key, uint64_t offset, uint64_t initial, time_t expiration, uint64_t *value, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const QByteArray _group = groupKey.toUtf8();
    const QByteArray _key = key.toUtf8();

    const memcached_return_t rt = memcached_decrement_with_initial_by_key(mcd->d_ptr->memc,
                                                                          _group.constData(),
                                                                          _group.size(),
                                                                          _key.constData(),
                                                                          _key.size(),
                                                                          offset,
                                                                          initial,
                                                                          expiration,
                                                                          value);

    const bool ok = memcached_success(rt);
    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to increment or initialize key \"%s\" in group \"%s\" by offset %lu or initial %lu: %s", _key.constData(), _group.constData(), offset, initial, memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::cas(const QString &key, const QByteArray &value, time_t expiration, uint64_t cas, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const QByteArray _key = key.toUtf8();

    MemcachedPrivate::Flags flags;
    QByteArray _value = value;

    if (mcd->d_ptr->compression && (_value.size() > mcd->d_ptr->compressionThreshold)) {
        flags |= MemcachedPrivate::Compressed;
        _value = qCompress(value, mcd->d_ptr->compressionLevel);
    }

    const memcached_return_t rt = memcached_cas(mcd->d_ptr->memc,
                                                _key.constData(),
                                                _key.size(),
                                                _value.constData(),
                                                _value.size(),
                                                expiration,
                                                flags,
                                                cas);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_DATA_EXISTS)) {
        qCWarning(C_MEMCACHED, "Failed to compare and set (cas) key \"%s\": %s", _key.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::casByKey(const QString &groupKey, const QString &key, const QByteArray &value, time_t expiration, uint64_t cas, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const QByteArray _group = groupKey.toUtf8();
    const QByteArray _key = key.toUtf8();

    MemcachedPrivate::Flags flags;
    QByteArray _value = value;

    if (mcd->d_ptr->compression && (_value.size() > mcd->d_ptr->compressionThreshold)) {
        flags |= MemcachedPrivate::Compressed;
        _value = qCompress(value, mcd->d_ptr->compressionLevel);
    }

    const memcached_return_t rt = memcached_cas_by_key(mcd->d_ptr->memc,
                                                       _group.constData(),
                                                       _group.size(),
                                                       _key.constData(),
                                                       _key.size(),
                                                       _value.constData(),
                                                       _value.size(),
                                                       expiration,
                                                       flags,
                                                       cas);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_DATA_EXISTS)) {
        qCWarning(C_MEMCACHED, "Failed to compare and set (cas) key \"%s\" in group \"%s\": %s", _key.constData(), _group.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::flushBuffers(MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const memcached_return_t rt = memcached_flush_buffers(mcd->d_ptr->memc);

    const bool ok = memcached_success(rt);

    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to flush buffers: %s", memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::flush(time_t expiration, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const memcached_return_t rt = memcached_flush(mcd->d_ptr->memc, expiration);

    const bool ok = memcached_success(rt);

    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to wipe clean (flush) server content: %s", memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

QHash<QString,QByteArray> Memcached::mget(const QStringList &keys, QHash<QString, uint64_t> *casValues, MemcachedReturnType *returnType)
{
    QHash<QString,QByteArray> ret;

    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return ret;
    }

    if (keys.empty()) {
        qCWarning(C_MEMCACHED, "Can not get multiple values without a list of keys.");
        if (returnType) {
            *returnType = Memcached::BadKeyProvided;
        }
        return ret;
    }

    std::vector<char *> _keys;
    _keys.reserve(keys.size());
    std::vector<size_t> _keysSizes;
    _keysSizes.reserve(keys.size());

    for (const QString &key : keys) {
        const QByteArray _key = key.toUtf8();
        char *data = new char[_key.size() + 1];
        strcpy(data, _key.data());
        _keys.push_back(data);
        _keysSizes.push_back(_key.size());
    }

    memcached_return_t rt;
    bool ok = false;

    rt = memcached_mget(mcd->d_ptr->memc,
                        &_keys[0],
                        &_keysSizes[0],
                        _keys.size());

    if (memcached_success(rt)) {
        ok = true;
        memcached_result_st *result;
        ret.reserve(keys.size());
        while ((rt != MEMCACHED_END) && (rt != MEMCACHED_NOTFOUND)) {
            result = memcached_fetch_result(mcd->d_ptr->memc, NULL, &rt);
            if (result) {
                const QString rk = QString::fromUtf8(memcached_result_key_value(result), memcached_result_key_length(result));
                QByteArray rd(memcached_result_value(result), memcached_result_length(result));
                if (casValues) {
                    casValues->insert(rk, memcached_result_cas(result));
                }
                MemcachedPrivate::Flags flags = MemcachedPrivate::Flags(memcached_result_flags(result));
                if (flags.testFlag(MemcachedPrivate::Compressed)) {
                    rd = qUncompress(rd);
                }
                ret.insert(rk, rd);
            }
            memcached_result_free(result);
        }
    }

    for (char *c : _keys) {
        delete [] c;
    }

    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to get values for multiple keys: %s", memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ret;
}

QHash<QString, QByteArray> Memcached::mgetByKey(const QString &groupKey, const QStringList &keys, QHash<QString, uint64_t> *casValues, MemcachedReturnType *returnType)
{
    QHash<QString, QByteArray> ret;

    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return ret;
    }

    if (groupKey.isEmpty()) {
        qCWarning(C_MEMCACHED, "Can not get multiple values from specific server when groupKey is empty.");
        if (returnType) {
            *returnType = Memcached::BadKeyProvided;
        }
        return ret;
    }

    if (keys.empty()) {
        qCWarning(C_MEMCACHED, "Can not get multiple values without a list of keys.");
        if (returnType) {
            *returnType = Memcached::BadKeyProvided;
        }
        return ret;
    }

    const QByteArray _group = groupKey.toUtf8();

    std::vector<char *> _keys;
    _keys.reserve(keys.size());
    std::vector<size_t> _keysSizes;
    _keysSizes.reserve(keys.size());

    for (const QString &key : keys) {
        const QByteArray _key = key.toUtf8();
        char *data = new char[_key.size() + 1];
        strcpy(data, _key.data());
        _keys.push_back(data);
        _keysSizes.push_back(_key.size());
    }

    memcached_return_t rt;
    bool ok = false;

    rt = memcached_mget_by_key(mcd->d_ptr->memc,
                               _group.constData(),
                               _group.size(),
                               &_keys[0],
                               &_keysSizes[0],
                               _keys.size());



    if (memcached_success(rt)) {
        ok = true;
        memcached_result_st *result;
        ret.reserve(keys.size());
        while ((rt != MEMCACHED_END) && (rt != MEMCACHED_NOTFOUND)) {
            result = memcached_fetch_result(mcd->d_ptr->memc, NULL, &rt);
            if (result) {
                const QString rk = QString::fromUtf8(memcached_result_key_value(result), memcached_result_key_length(result));
                QByteArray rd(memcached_result_value(result), memcached_result_length(result));
                if (casValues) {
                    casValues->insert(rk, memcached_result_cas(result));
                }
                MemcachedPrivate::Flags flags = MemcachedPrivate::Flags(memcached_result_flags(result));
                if (flags.testFlag(MemcachedPrivate::Compressed)) {
                    rd = qUncompress(rd);
                }
                ret.insert(rk, rd);
            }
            memcached_result_free(result);
        }
    }

    for (char *c : _keys) {
        delete [] c;
    }

    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to get values for multiple keys in group \"%s\": %s", _group.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ret;
}

bool Memcached::touch(const QString &key, time_t expiration, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const QByteArray _key = key.toUtf8();

    const memcached_return_t rt = memcached_touch(mcd->d_ptr->memc,
                                                  _key.constData(),
                                                  _key.size(),
                                                  expiration);

    const bool ok = memcached_success(rt);

    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to touch key \"%s\" with new expiration time %lu: %s", _key.constData(), expiration, memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::touchByKey(const QString &groupKey, const QString &key, time_t expiration, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const QByteArray _group = groupKey.toUtf8();
    const QByteArray _key = key.toUtf8();

    const memcached_return_t rt = memcached_touch_by_key(mcd->d_ptr->memc,
                                                         _group.constData(),
                                                         _group.size(),
                                                         _key.constData(),
                                                         _key.size(),
                                                         expiration);

    const bool ok = memcached_success(rt);

    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to touch key \"%s\" in group \"%s\" with new expiration time %lu: %s", _key.constData(), _group.constData(), expiration, memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

QString Memcached::errorString(Context *c, MemcachedReturnType rt)
{
    switch(rt) {
    case Memcached::Success:
        return c->translate("Cutelyst::Memcached", "The request was successfully executed.");
    case Memcached::Failure:
        return c->translate("Cutelyst::Memcached", "An unknown failure has occurred in the server.");
    case Memcached::HostLookupFailure:
        return c->translate("Cutelyst::Memcached", "A DNS failure has occurred.");
    case Memcached::ConnectionFailure:
        return c->translate("Cutelyst::Memcached", "An unknown error has occured while trying to connect to a server.");
    case Memcached::WriteFailure:
        return c->translate("Cutelyst::Memcached", "An error has occured while trying to write to a server.");
    case Memcached::ReadFailure:
        return c->translate("Cutelyst::Memcached", "An error has occured while trying to read from a server.");
    case Memcached::UnknownReadFailure:
        return c->translate("Cutelyst::Memcached", "An unknown error has occured while trying to read from a server. This only occures when either there is a bug in the server, or in rare cases where an ethernet NIC is reporting dubious information.");
    case Memcached::ProtocolError:
        return c->translate("Cutelyst::Memcached", "An unknown error has occured in the protocol.");
    case Memcached::ClientError:
        return c->translate("Cutelyst::Memcached", "An unknown client error has occured internally.");
    case Memcached::ServerError:
        return c->translate("Cutelyst::Memcached", "An unknown error has occured in the server.");
    case Memcached::Error:
        return c->translate("Cutelyst::Memcached", "A general error occured.");
    case Memcached::DataExists:
        return c->translate("Cutelyst::Memcached", "The data for the given key alrey exists.");
    case Memcached::DataDoesNotExist:
        return c->translate("Cutelyst::Memcached", "The data requested with the key given was not found.");
    case Memcached::NotStored:
        return c->translate("Cutelyst::Memcached", "The request to store an object failed.");
    case Memcached::Stored:
        return c->translate("Cutelyst::Memcached", "The requested object has been successfully stored on the server.");
    case Memcached::NotFound:
        return c->translate("Cutelyst::Memcached", "The object requested was not found.");
    case Memcached::MemoryAllocationFailure:
        return c->translate("Cutelyst::Memcached", "An error has occurred while trying to allocate memory.");
    case Memcached::PartialRead:
        return c->translate("Cutelyst::Memcached", "The read operation was only partcially successful.");
    case Memcached::SomeErrors:
        return c->translate("Cutelyst::Memcached", "A multi request has been made, and some underterminate number of errors have occurred.");
    case Memcached::NoServers:
        return c->translate("Cutelyst::Memcached", "No servers have been added to the Memcached plugin.");
    case Memcached::End:
        return c->translate("Cutelyst::Memcached", "The server has completed returning all of the objects requested.");
    case Memcached::Deleted:
        return c->translate("Cutelyst::Memcached", "The object requested by the key has been deleted.");
    case Memcached::Stat:
        return c->translate("Cutelyst::Memcached", "A \"stat\" command has been returned in the protocol.");
    case Memcached::Errno:
        return c->translate("Cutelyst::Memcached", "An error has occurred in the driver which has set errno.");
    case Memcached::NotSupported:
        return c->translate("Cutelyst::Memcached", "The given method is not supported in the server.");
    case Memcached::FetchNotFinished:
        return c->translate("Cutelyst::Memcached", "A request has been made, but the server has not finished the fetch of the last request.");
    case Memcached::Timeout:
        return c->translate("Cutelyst::Memcached", "The operation has timed out.");
    case Memcached::Buffered:
        return c->translate("Cutelyst::Memcached", "The request has been buffered.");
    case Memcached::BadKeyProvided:
        return c->translate("Cutelyst::Memcached", "The key provided is not a valid key.");
    case Memcached::InvalidHostProtocol:
        return c->translate("Cutelyst::Memcached", "The server you are connecting to has an invalid protocol. Most likely you are connecting to an older server that does not speak the binary protocol.");
    case Memcached::ServerMarkedDead:
        return c->translate("Cutelyst::Memcached", "The requested server has been marked dead.");
    case Memcached::UnknownStatKey:
        return c->translate("Cutelyst::Memcached", "The server you are communicating with has a stat key which has not be defined in the protocol.");
    case Memcached::E2Big:
        return c->translate("Cutelyst::Memcached", "Item is too large for the server to store.");
    case Memcached::InvalidArguments:
        return c->translate("Cutelyst::Memcached", "The arguments supplied to the given function were not valid.");
    case Memcached::KeyTooBig:
        return c->translate("Cutelyst::Memcached", "The key that has been provided is too large for the given server.");
    case Memcached::AuthProblem:
        return c->translate("Cutelyst::Memcached", "An unknown issue has occured during SASL authentication.");
    case Memcached::AuthFailure:
        return c->translate("Cutelyst::Memcached", "The credentials provided are not valid for this server.");
    case Memcached::AuthContinue:
        return c->translate("Cutelyst::Memcached", "Authentication has been paused.");
    case Memcached::ParseError:
        return c->translate("Cutelyst::Memcached", "An error has occurred while trying to parse the configuration string.");
    case Memcached::ParseUserError:
        return c->translate("Cutelyst::Memcached", "An error has occurred in parsing the configuration string.");
    case Memcached::Deprecated:
        return c->translate("Cutelyst::Memcached", "The method that was requested has been deprecated.");
    case Memcached::PluginNotRegisterd:
        return c->translate("Cutelyst::Memcached", "The Cutelyst Memcached plugin has not been registered to the application.");
    default:
        return c->translate("Cutelyst::Memcached", "An unknown error has occured.");
    }
}

void MemcachedPrivate::_q_postFork(Application *app)
{
    mcd = app->plugin<Memcached *>();
}

Memcached::MemcachedReturnType MemcachedPrivate::returnTypeConvert(memcached_return_t rt)
{
    switch (rt) {
    case MEMCACHED_SUCCESS:                             return Memcached::Success;
    case MEMCACHED_FAILURE:                             return Memcached::Failure;
    case MEMCACHED_HOST_LOOKUP_FAILURE:                 return Memcached::HostLookupFailure;
    case MEMCACHED_CONNECTION_FAILURE:                  return Memcached::ConnectionFailure;
    case MEMCACHED_CONNECTION_BIND_FAILURE:             return Memcached::HostLookupFailure;
    case MEMCACHED_WRITE_FAILURE:                       return Memcached::WriteFailure;
    case MEMCACHED_READ_FAILURE:                        return Memcached::ReadFailure;
    case MEMCACHED_UNKNOWN_READ_FAILURE:                return Memcached::UnknownReadFailure;
    case MEMCACHED_PROTOCOL_ERROR:                      return Memcached::ProtocolError;
    case MEMCACHED_CLIENT_ERROR:                        return Memcached::ClientError;
    case MEMCACHED_SERVER_ERROR:                        return Memcached::ServerError;
    case MEMCACHED_ERROR:                               return Memcached::Error;
    case MEMCACHED_DATA_EXISTS:                         return Memcached::DataExists;
    case MEMCACHED_DATA_DOES_NOT_EXIST:                 return Memcached::DataDoesNotExist;
    case MEMCACHED_NOTSTORED:                           return Memcached::NotStored;
    case MEMCACHED_STORED:                              return Memcached::Stored;
    case MEMCACHED_NOTFOUND:                            return Memcached::NotFound;
    case MEMCACHED_MEMORY_ALLOCATION_FAILURE:           return Memcached::MemoryAllocationFailure;
    case MEMCACHED_PARTIAL_READ:                        return Memcached::PartialRead;
    case MEMCACHED_SOME_ERRORS:                         return Memcached::SomeErrors;
    case MEMCACHED_NO_SERVERS:                          return Memcached::NoServers;
    case MEMCACHED_END:                                 return Memcached::End;
    case MEMCACHED_DELETED:                             return Memcached::Deleted;
    case MEMCACHED_STAT:                                return Memcached::Stat;
    case MEMCACHED_ERRNO:                               return Memcached::Errno;
    case MEMCACHED_NOT_SUPPORTED:                       return Memcached::NotSupported;
    case MEMCACHED_FETCH_NOTFINISHED:                   return Memcached::FetchNotFinished;
    case MEMCACHED_TIMEOUT:                             return Memcached::Timeout;
    case MEMCACHED_BUFFERED:                            return Memcached::Buffered;
    case MEMCACHED_BAD_KEY_PROVIDED:                    return Memcached::BadKeyProvided;
    case MEMCACHED_INVALID_HOST_PROTOCOL:               return Memcached::InvalidHostProtocol;
    case MEMCACHED_SERVER_MARKED_DEAD:                  return Memcached::ServerMarkedDead;
    case MEMCACHED_UNKNOWN_STAT_KEY:                    return Memcached::UnknownStatKey;
    case MEMCACHED_E2BIG:                               return Memcached::E2Big;
    case MEMCACHED_INVALID_ARGUMENTS:                   return Memcached::InvalidArguments;
    case MEMCACHED_KEY_TOO_BIG:                         return Memcached::KeyTooBig;
    case MEMCACHED_AUTH_PROBLEM:                        return Memcached::AuthProblem;
    case MEMCACHED_AUTH_FAILURE:                        return Memcached::AuthFailure;
    case MEMCACHED_AUTH_CONTINUE:                       return Memcached::AuthContinue;
    case MEMCACHED_PARSE_ERROR:                         return Memcached::ParseError;
    case MEMCACHED_PARSE_USER_ERROR:                    return Memcached::ParseUserError;
    case MEMCACHED_DEPRECATED:                          return Memcached::Deprecated;
    case MEMCACHED_IN_PROGRESS:                         return Memcached::InProgress;
    case MEMCACHED_SERVER_TEMPORARILY_DISABLED:         return Memcached::ServerTemporaryDisabled;
    case MEMCACHED_SERVER_MEMORY_ALLOCATION_FAILURE:    return Memcached::ServerMemoryAllocationFailure;
    case MEMCACHED_MAXIMUM_RETURN:                      return Memcached::MaximumReturn;
    default:                                            return Memcached::Success;
    }
}

void MemcachedPrivate::setReturnType(Memcached::MemcachedReturnType *rt1, memcached_return_t rt2)
{
    if (rt1) {
        *rt1 = MemcachedPrivate::returnTypeConvert(rt2);
    }
}

#include "moc_memcached.cpp"
