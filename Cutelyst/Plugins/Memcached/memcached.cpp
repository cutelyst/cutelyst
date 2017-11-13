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
        if (map.value(flag, false).toBool()) {
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
        if (map.contains(opt)) {
            const QString optStr = QLatin1String("--") + opt.toUpper().replace(QLatin1Char('_'), QLatin1Char('-')) + QLatin1Char('=') + map.value(opt).toString();
            config.push_back(optStr);
        }
    }

    d->compression = map.value(QStringLiteral("compression"), false).toBool();
    d->compressionLevel = map.value(QStringLiteral("compression_level"), -1).toInt();
    d->compressionThreshold = map.value(QStringLiteral("compression_threshold"), 100).toInt();
    qCDebug(C_MEMCACHED, "Compression: %s, Compression level: %i, Compression threshold: %i bytes", d->compression ? "enabled" : "disabled", d->compressionLevel, d->compressionThreshold);

    const QByteArray configString = config.join(QChar(QChar::Space)).toUtf8();

    bool ok = false;

    qCDebug(C_MEMCACHED, "Setting up connection to memcached servers using libmemcached %s with the following configuration string: \"%s\"", memcached_lib_version(), configString.constData());

    memcached_st *new_memc = memcached(configString.constData(), configString.size());

    if (new_memc) {
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
    if (!MemcachedPrivate::checkInput(mcd, key, QStringLiteral("set"), returnType)) {
        return false;
    }

    const QByteArray _key = key.toUtf8();

    MemcachedPrivate::Flags flags;
    QByteArray _value = value;

    if (mcd->d_ptr->compression && (_value.size() > mcd->d_ptr->compressionThreshold)) {
        flags.setFlag(MemcachedPrivate::Compressed);
        _value = qCompress(value, mcd->d_ptr->compressionLevel);
    }

    const memcached_return_t rt = memcached_set(mcd->d_ptr->memc,
                                                _key.constData(),
                                                _key.size(),
                                                _value.constData(),
                                                _value.size(),
                                                expiration,
                                                flags);

    const bool ok = (rt == MEMCACHED_SUCCESS);

    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to store key \"%s\": %s", _key.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::setByKey(const QString &groupKey, const QString &key, const QByteArray &value, time_t expiration, MemcachedReturnType *returnType)
{
    if (!MemcachedPrivate::checkInputByKey(mcd, groupKey, key, QStringLiteral("set"), returnType)) {
        return false;
    }

    const QByteArray _key = key.toUtf8();
    const QByteArray _groupKey = groupKey.toUtf8();

    MemcachedPrivate::Flags flags;
    QByteArray _value = value;

    if (mcd->d_ptr->compression && (_value.size() > mcd->d_ptr->compressionThreshold)) {
        flags.setFlag(MemcachedPrivate::Compressed);
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

    const bool ok = (rt == MEMCACHED_SUCCESS);

    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to store key \"%s\" on group \"%s\": %s", _key.constData(), _groupKey.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::add(const QString &key, const QByteArray &value, time_t expiration, MemcachedReturnType *returnType)
{
    if (!MemcachedPrivate::checkInput(mcd, key, QStringLiteral("add"), returnType)) {
        return false;
    }

    const QByteArray _key = key.toUtf8();

    MemcachedPrivate::Flags flags;
    QByteArray _value = value;

    if (mcd->d_ptr->compression && (_value.size() > mcd->d_ptr->compressionThreshold)) {
        flags.setFlag(MemcachedPrivate::Compressed);
        _value = qCompress(value, mcd->d_ptr->compressionLevel);
    }

    const memcached_return_t rt = memcached_add(mcd->d_ptr->memc,
                                                _key.constData(),
                                                _key.size(),
                                                _value.constData(),
                                                _value.size(),
                                                expiration,
                                                flags);

    const bool ok = (rt == MEMCACHED_SUCCESS);

    if (!ok && (rt != MEMCACHED_NOTSTORED)) {
        qCWarning(C_MEMCACHED, "Failed to add key \"%s\": %s", _key.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::addByKey(const QString &groupKey, const QString &key, const QByteArray &value, time_t expiration, MemcachedReturnType *returnType)
{
    if (!MemcachedPrivate::checkInputByKey(mcd, groupKey, key, QStringLiteral("add"), returnType)) {
        return false;
    }

    const QByteArray _key = key.toUtf8();
    const QByteArray _groupKey = groupKey.toUtf8();

    MemcachedPrivate::Flags flags;
    QByteArray _value = value;

    if (mcd->d_ptr->compression && (_value.size() > mcd->d_ptr->compressionThreshold)) {
        flags.setFlag(MemcachedPrivate::Compressed);
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

    const bool ok = (rt == MEMCACHED_SUCCESS);

    if (!ok && (rt != MEMCACHED_NOTSTORED)) {
        qCWarning(C_MEMCACHED, "Failed to add key \"%s\" on group \"%s\": %s", _key.constData(), _groupKey.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::replace(const QString &key, const QByteArray &value, time_t expiration, MemcachedReturnType *returnType)
{
    if (!MemcachedPrivate::checkInput(mcd, key, QStringLiteral("replace"), returnType)) {
        return false;
    }

    const QByteArray _key = key.toUtf8();

    MemcachedPrivate::Flags flags;
    QByteArray _value = value;

    if (mcd->d_ptr->compression && (_value.size() > mcd->d_ptr->compressionThreshold)) {
        flags.setFlag(MemcachedPrivate::Compressed);
        _value = qCompress(value, mcd->d_ptr->compressionLevel);
    }

    const memcached_return_t rt = memcached_replace(mcd->d_ptr->memc,
                                                    _key.constData(),
                                                    _key.size(),
                                                    _value.constData(),
                                                    _value.size(),
                                                    expiration,
                                                    flags);

    const bool ok = (rt == MEMCACHED_SUCCESS);

    if (!ok && (rt != MEMCACHED_NOTSTORED)) {
        qCWarning(C_MEMCACHED, "Failed to replace key \"%s\": %s", _key.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::replaceByKey(const QString &groupKey, const QString &key, const QByteArray &value, time_t expiration, MemcachedReturnType *returnType)
{
    if (!MemcachedPrivate::checkInputByKey(mcd, groupKey, key, QStringLiteral("replace"), returnType)) {
        return false;
    }

    const QByteArray _groupKey = groupKey.toUtf8();
    const QByteArray _key = key.toUtf8();

    MemcachedPrivate::Flags flags;
    QByteArray _value = value;

    if (mcd->d_ptr->compression && (_value.size() > mcd->d_ptr->compressionThreshold)) {
        flags.setFlag(MemcachedPrivate::Compressed);
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

    const bool ok = (rt == MEMCACHED_SUCCESS);

    if (!ok && (rt != MEMCACHED_NOTSTORED)) {
        qCWarning(C_MEMCACHED, "Failed to replace key \"%s\" on group \"%s\": %s", _key.constData(), _groupKey.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

QByteArray Memcached::get(const QString &key, quint64 *cas, Cutelyst::Memcached::MemcachedReturnType *returnType)
{
    QByteArray retData;

    if (!MemcachedPrivate::checkInput(mcd, key, QStringLiteral("get"), returnType)) {
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

    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to get data for key \"%s\": %s", _key.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return retData;
}

QByteArray Memcached::getByKey(const QString &groupKey, const QString &key, quint64 *cas, MemcachedReturnType *returnType)
{
    QByteArray retData;

    if (!MemcachedPrivate::checkInputByKey(mcd, groupKey, key, QStringLiteral("get"), returnType)) {
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

    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to get data for key \"%s\" on group \"%s\": %s", _key.constData(), _groupKey.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return retData;
}

bool Memcached::remove(const QString &key, quint32 expiration, MemcachedReturnType *returnType)
{
    if (!MemcachedPrivate::checkInput(mcd, key, QStringLiteral("remove"), returnType)) {
        return false;
    }

    const QByteArray _key = key.toUtf8();

    const memcached_return_t rt = memcached_delete(mcd->d_ptr->memc,
                                                   _key.constData(),
                                                   _key.size(),
                                                   expiration);

    const bool ok = (rt == MEMCACHED_SUCCESS);

    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to remove data for key \"%s\": %s", _key.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::remove(const QString &key, MemcachedReturnType *returnType)
{
    return Memcached::remove(key, 0, returnType);
}

bool Memcached::removeByKey(const QString &groupKey, const QString &key, quint32 expiration, MemcachedReturnType *returnType)
{
    if (!MemcachedPrivate::checkInputByKey(mcd, groupKey, key, QStringLiteral("remove"), returnType)) {
        return false;
    }

    const QByteArray _groupKey = groupKey.toUtf8();
    const QByteArray _key = key.toUtf8();

    const memcached_return_t rt = memcached_delete_by_key(mcd->d_ptr->memc,
                                                          _groupKey.constData(),
                                                          _groupKey.size(),
                                                          _key.constData(),
                                                          _key.size(),
                                                          expiration);

    const bool ok = (rt == MEMCACHED_SUCCESS);

    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to remove data for key \"%s\" on group \"%s\": %s", _key.constData(), _groupKey.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    return ok;
}

bool Memcached::removeByKey(const QString &groupKey, const QString &key, MemcachedReturnType *returnType)
{
    return Memcached::removeByKey(groupKey, key, 0, returnType);
}

bool Memcached::exist(const QString &key, MemcachedReturnType *returnType)
{
    if (!MemcachedPrivate::checkInput(mcd, key, QStringLiteral("check for existing"), returnType)) {
        return false;
    }

    const QByteArray _key = key.toUtf8();

    const memcached_return_t rt = memcached_exist(mcd->d_ptr->memc,
                                                  _key.constData(),
                                                  _key.size());

    const bool ok = (rt == MEMCACHED_SUCCESS);

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED, "Failed to check existence of key \"%s\": %s", _key.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::existByKey(const QString &groupKey, const QString &key, MemcachedReturnType *returnType)
{
    if (!MemcachedPrivate::checkInputByKey(mcd, groupKey, key, QStringLiteral("check for existing"), returnType)) {
        return false;
    }

    const QByteArray _groupKey = groupKey.toUtf8();
    const QByteArray _key = key.toUtf8();

    const memcached_return_t rt = memcached_exist_by_key(mcd->d_ptr->memc,
                                                         _groupKey.constData(),
                                                         _groupKey.size(),
                                                         _key.constData(),
                                                         _key.size());

    const bool ok = (rt == MEMCACHED_SUCCESS);

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED, "Failed to check existence of key \"%s\" in group \"%s\": %s", _key.constData(), _groupKey.constData());
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::increment(const QString &key, uint32_t offset, uint64_t *value, MemcachedReturnType *returnType)
{
    if (!MemcachedPrivate::checkInput(mcd, key, QStringLiteral("increment"), returnType)) {
        return false;
    }

    const QByteArray _key = key.toUtf8();

    const memcached_return_t rt = memcached_increment(mcd->d_ptr->memc,
                                                      _key.constData(),
                                                      _key.size(),
                                                      offset,
                                                      value);

    const bool ok = (rt == MEMCACHED_SUCCESS);

    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to increment key \"%s\" by %lu: %s", _key.constData(), offset, memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::incrementByKey(const QString &groupKey, const QString &key, uint64_t offset, uint64_t *value, MemcachedReturnType *returnType)
{
    if (!MemcachedPrivate::checkInputByKey(mcd, groupKey, key, QStringLiteral("increment"), returnType)) {
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

    const bool ok = (rt == MEMCACHED_SUCCESS);

    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to increment \"%s\" key on group \"%s\" by %lu: %s", _key.constData(), _group.constData(), offset, memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::incrementWithInitial(const QString &key, uint64_t offset, uint64_t initial, time_t expiration, uint64_t *value, MemcachedReturnType *returnType)
{
    if (!MemcachedPrivate::checkInput(mcd, key, QStringLiteral("increment with initial"), returnType)) {
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

    const bool ok = (rt == MEMCACHED_SUCCESS);

    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to increment or initialize key \"%s\" by offset %lu or initial %lu: %s", _key.constData(), offset, initial, memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::incrementWithInitialByKey(const QString &groupKey, const QString &key, uint64_t offset, uint64_t initial, time_t expiration, uint64_t *value, MemcachedReturnType *returnType)
{
    if (!MemcachedPrivate::checkInputByKey(mcd, groupKey, key, QStringLiteral("increment or initialize"), returnType)) {
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

    const bool ok = (rt == MEMCACHED_SUCCESS);
    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to increment or initialize key \"%s\" in group \"%s\" by offset %lu or initial %lu: %s", _key.constData(), _group.constData(), offset, initial, memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::decrement(const QString &key, uint32_t offset, uint64_t *value, MemcachedReturnType *returnType)
{
    if (!MemcachedPrivate::checkInput(mcd, key, QStringLiteral("decrement"), returnType)) {
        return false;
    }

    const QByteArray _key = key.toUtf8();

    const memcached_return_t rt = memcached_decrement(mcd->d_ptr->memc,
                                                      _key.constData(),
                                                      _key.size(),
                                                      offset,
                                                      value);

    const bool ok = (rt == MEMCACHED_SUCCESS);

    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to decrement key \"%s\" by %lu: %s", _key.constData(), offset, memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::decrementByKey(const QString &groupKey, const QString &key, uint64_t offset, uint64_t *value, MemcachedReturnType *returnType)
{
    if (!MemcachedPrivate::checkInputByKey(mcd, groupKey, key, QStringLiteral("decrement"), returnType)) {
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

    const bool ok = (rt == MEMCACHED_SUCCESS);

    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to decrement \"%s\" key on group \"%s\" by %lu: %s", _key.constData(), _group.constData(), offset, memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::decrementWithInitial(const QString &key, uint64_t offset, uint64_t initial, time_t expiration, uint64_t *value, MemcachedReturnType *returnType)
{
    if (!MemcachedPrivate::checkInput(mcd, key, QStringLiteral("decrement with initial"), returnType)) {
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

    const bool ok = (rt == MEMCACHED_SUCCESS);

    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to decrement or initialize key \"%s\" by offset %lu or initial %lu: %s", _key.constData(), offset, initial, memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::decrementWithInitialByKey(const QString &groupKey, const QString &key, uint64_t offset, uint64_t initial, time_t expiration, uint64_t *value, MemcachedReturnType *returnType)
{
    if (!MemcachedPrivate::checkInputByKey(mcd, groupKey, key, QStringLiteral("decrement or initialize"), returnType)) {
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

    const bool ok = (rt == MEMCACHED_SUCCESS);
    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to increment or initialize key \"%s\" in group \"%s\" by offset %lu or initial %lu: %s", _key.constData(), _group.constData(), offset, initial, memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::cas(const QString &key, const QByteArray &value, time_t expiration, quint64 cas, MemcachedReturnType *returnType)
{
    if (!MemcachedPrivate::checkInput(mcd, key, QStringLiteral("compare and set (cas)"), returnType)) {
        return false;
    }

    const QByteArray _key = key.toUtf8();

    MemcachedPrivate::Flags flags;
    QByteArray _value = value;

    if (mcd->d_ptr->compression && (_value.size() > mcd->d_ptr->compressionThreshold)) {
        flags.setFlag(MemcachedPrivate::Compressed);
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

    const bool ok = (rt == MEMCACHED_SUCCESS);

    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to compare and set (cas) key \"%s\": %s", _key.constData(), memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::casByKey(const QString &groupKey, const QString &key, const QByteArray &value, time_t expiration, quint64 cas, MemcachedReturnType *returnType)
{
    if (!MemcachedPrivate::checkInputByKey(mcd, groupKey, key, QStringLiteral("compare and set (cas)"), returnType)) {
        return false;
    }

    const QByteArray _group = groupKey.toUtf8();
    const QByteArray _key = key.toUtf8();

    MemcachedPrivate::Flags flags;
    QByteArray _value = value;

    if (mcd->d_ptr->compression && (_value.size() > mcd->d_ptr->compressionThreshold)) {
        flags.setFlag(MemcachedPrivate::Compressed);
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

    const bool ok = (rt == MEMCACHED_SUCCESS);

    if (!ok) {
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
            *returnType = Memcached::Error;
        }
        return false;
    }

    const memcached_return_t rt = memcached_flush_buffers(mcd->d_ptr->memc);

    const bool ok = (rt == MEMCACHED_SUCCESS);

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
            *returnType = Memcached::Error;
        }
        return false;
    }

    const memcached_return_t rt = memcached_flush(mcd->d_ptr->memc, expiration);

    const bool ok = (rt == MEMCACHED_SUCCESS);

    if (!ok) {
        qCWarning(C_MEMCACHED, "Failed to wipe clean (flush) server content: %s", memcached_strerror(mcd->d_ptr->memc, rt));
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
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

bool MemcachedPrivate::checkInput(Memcached *ptr, const QString &key, const QString &action, Memcached::MemcachedReturnType *rt)
{
    if (!ptr) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (rt) {
            *rt = Memcached::Error;
        }
        return false;
    }

    if (key.isEmpty()) {
        qCWarning(C_MEMCACHED) << "Can not" << action << "data without a valid key name";
        if (rt) {
            *rt = Memcached::BadKeyProvided;
        }
        return false;
    }

    return true;
}

bool MemcachedPrivate::checkInputByKey(Memcached *ptr, const QString &groupKey, const QString &key, const QString &action, Memcached::MemcachedReturnType *rt)
{
    if (!ptr) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (rt) {
            *rt = Memcached::Error;
        }
        return false;
    }

    if (key.isEmpty()) {
        qCWarning(C_MEMCACHED) << "Can not" << action << "data without a valid key name";
        if (rt) {
            *rt = Memcached::BadKeyProvided;
        }
        return false;
    }

    if (groupKey.isEmpty()) {
        qCWarning(C_MEMCACHED) << "Can not" << action << "data without a valid group key name";
        if (rt) {
            *rt = Memcached::BadKeyProvided;
        }
        return false;
    }

    return true;
}

void MemcachedPrivate::setReturnType(Memcached::MemcachedReturnType *rt1, memcached_return_t rt2)
{
    if (rt1) {
        *rt1 = MemcachedPrivate::returnTypeConvert(rt2);
    }
}

#include "moc_memcached.cpp"
