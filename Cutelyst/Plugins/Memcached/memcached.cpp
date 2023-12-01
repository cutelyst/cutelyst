/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "memcached_p.h"

#include <Cutelyst/Application>
#include <Cutelyst/Context>
#include <Cutelyst/Engine>

#include <QLoggingCategory>
#include <QStringList>

Q_LOGGING_CATEGORY(C_MEMCACHED, "cutelyst.plugin.memcached", QtWarningMsg)

using namespace Cutelyst;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static thread_local Memcached *mcd = nullptr;
const time_t Memcached::expirationNotAdd{static_cast<time_t>(MEMCACHED_EXPIRATION_NOT_ADD)};
const std::chrono::seconds Memcached::expirationNotAddDuration{
    static_cast<std::chrono::seconds::rep>(MEMCACHED_EXPIRATION_NOT_ADD)};

Memcached::Memcached(Application *parent)
    : Plugin(parent)
    , d_ptr(new MemcachedPrivate)
{
}

Memcached::Memcached(Application *parent, const QVariantMap &defaultConfig)
    : Plugin(parent)
    , d_ptr(new MemcachedPrivate(defaultConfig))
{
}

Memcached::~Memcached() = default;

void Memcached::setDefaultConfig(const QVariantMap &defaultConfig)
{
    Q_D(Memcached);
    d->defaultConfig = defaultConfig;
}

bool Memcached::setup(Application *app)
{
    Q_D(Memcached);

    d->loadedConfig = app->engine()->config(u"Cutelyst_Memcached_Plugin"_qs);
    QStringList memcConfig;

    const QStringList serverList = d->config(u"servers"_qs).toString().split(u';');

    if (serverList.empty()) {
        memcConfig.push_back(u"--SERVER=localhost"_qs);
    }

    for (const QString &flag : {u"verify_key"_qs,
                                u"remove_failed_servers"_qs,
                                u"binary_protocol"_qs,
                                u"buffer_requests"_qs,
                                u"hash_with_namespace"_qs,
                                u"noreply"_qs,
                                u"randomize_replica_read"_qs,
                                u"sort_hosts"_qs,
                                u"support_cas"_qs,
                                u"use_udp"_qs,
                                u"tcp_nodelay"_qs,
                                u"tcp_keepalive"_qs}) {
        if (d->config(flag, false).toBool()) {
            const QString flagStr = u"--" + flag.toUpper().replace(u'_', u'-');
            memcConfig.push_back(flagStr);
        }
    }

    const bool useUDP = d->config(u"use_udp"_qs, false).toBool();

    for (const QString &opt : {
             u"connect_timeout"_qs,
             u"distribution"_qs,
             u"hash"_qs,
             u"number_of_replicas"_qs,
             u"namespace"_qs,
             u"retry_timeout"_qs,
             u"server_failure_limit"_qs,
             u"snd_timeout"_qs,
             u"socket_recv_size"_qs,
             u"socket_send_size"_qs,
             u"poll_timeout"_qs,
             u"io_bytes_watermark"_qs,
             u"io_key_prefetch"_qs,
             u"io_msg_watermark"_qs,
             u"rcv_timeout"_qs,
         }) {
        const QString _val = d->config(opt).toString();
        if (!_val.isEmpty()) {
            const QString optStr = u"--" + opt.toUpper().replace(u'_', u'-') + u'=' + _val;
            memcConfig.push_back(optStr); // clazy:exclude=reserve-candidates
        }
    }

    const QByteArray configString = memcConfig.join(u' ').toUtf8();

    bool ok = false;

    qCInfo(C_MEMCACHED) << "Setting up connection to memcached servers using libmemcached"
                        << memcached_lib_version()
                        << "with the following configuration string:" << configString;

    memcached_st *new_memc = memcached(configString.constData(), configString.size());

    if (new_memc) {

        if (!serverList.empty()) {
            for (const QString &server : serverList) {
                const auto serverParts = QStringView(server).split(u',');
                QString name;
                uint16_t port   = MemcachedPrivate::defaultPort;
                uint32_t weight = 1;
                bool isSocket   = false;
                if (!serverParts.empty()) {
                    const auto part0 = serverParts.at(0);
                    if (!part0.isEmpty()) {
                        name     = part0.toString();
                        isSocket = name.startsWith(u'/');
                    }
                    if (serverParts.size() > 1) {
                        const auto part1 = serverParts.at(1);
                        if (!part1.isEmpty()) {
                            if (isSocket) {
                                weight = part1.toUInt();
                            } else {
                                port = part1.toUInt();
                            }
                        }
                        if (!isSocket && (serverParts.size() > 2)) {
                            const auto part2 = serverParts.at(2);
                            if (!part2.isEmpty()) {
                                weight = part2.toUInt();
                            }
                        }
                    }
                }
                if (!name.isEmpty()) {
                    memcached_return_t rc{MEMCACHED_FAILURE};
                    if (isSocket) {
                        rc = memcached_server_add_unix_socket_with_weight(
                            new_memc, name.toUtf8().constData(), weight);
                        if (Q_LIKELY(memcached_success(rc))) {
                            qCInfo(C_MEMCACHED) << "Added memcached server on socket" << name
                                                << "with weight" << weight;
                        } else {
                            qCWarning(C_MEMCACHED).nospace()
                                << "Failed to add memcached server on socket " << name
                                << " with weight " << weight << ": "
                                << memcached_strerror(new_memc, rc);
                        }
                    } else {
                        if (useUDP) {
                            rc = memcached_server_add_udp_with_weight(
                                new_memc, name.toUtf8().constData(), port, weight);
                        } else {
                            rc = memcached_server_add_with_weight(
                                new_memc, name.toUtf8().constData(), port, weight);
                        }
                        if (Q_LIKELY(memcached_success(rc))) {
                            qCInfo(C_MEMCACHED).nospace().noquote()
                                << "Added memcached server on host " << name << ":" << port
                                << " with weight" << weight;
                        } else {
                            qCWarning(C_MEMCACHED).nospace().noquote()
                                << "Failed to add memcached server no host " << name << ":" << port
                                << " with weight " << weight << ": "
                                << memcached_strerror(new_memc, rc);
                        }
                    }
                }
            }

            if (Q_UNLIKELY(memcached_server_count(new_memc) == 0)) {
                qCWarning(C_MEMCACHED)
                    << "Failed to add any memcached server. Adding default server on localhost"
                    << "port 11211.";
                memcached_return_t rc =
                    memcached_server_add(new_memc, "localhost", MemcachedPrivate::defaultPort);
                if (Q_UNLIKELY(!memcached_success(rc))) {
                    qCCritical(C_MEMCACHED)
                        << "Failed to add default memcached server. Memcached plugin will not"
                        << "work without a configured server!" << memcached_strerror(new_memc, rc);
                    memcached_free(new_memc);
                    return false;
                }
            }
        }

        d->compression      = d->config(u"compression"_qs, false).toBool();
        d->compressionLevel = d->config(u"compression_level"_qs, -1).toInt();
        d->compressionThreshold =
            d->config(u"compression_threshold"_qs, MemcachedPrivate::defaultCompressionThreshold)
                .toInt();
        if (d->compression) {
            qCInfo(C_MEMCACHED).nospace()
                << "Compression: enabled (Compression level: " << d->compressionLevel
                << ", Compression threshold: " << d->compressionThreshold << " bytes";
        } else {
            qCInfo(C_MEMCACHED) << "Compression: disabled";
        }

        const QString encKey = d->config(u"encryption_key"_qs).toString();
        if (!encKey.isEmpty()) {
            const QByteArray encKeyBa = encKey.toUtf8();
            const memcached_return_t rt =
                memcached_set_encoding_key(new_memc, encKeyBa.constData(), encKeyBa.size());
            if (Q_LIKELY(memcached_success(rt))) {
                qCInfo(C_MEMCACHED) << "Encryption: enabled";
            } else {
                qCWarning(C_MEMCACHED)
                    << "Failed to enable encryption:" << memcached_strerror(new_memc, rt);
            }
        } else {
            qCInfo(C_MEMCACHED) << "Encryption: disabled";
        }

#ifdef LIBMEMCACHED_WITH_SASL_SUPPORT
#    if LIBMEMCACHED_WITH_SASL_SUPPORT == 1
        const QString saslUser = d->config(u"sasl_user"_qs).toString();
        const QString saslPass = d->config(u"sasl_password"_qs).toString();
        if (!saslUser.isEmpty() && !saslPass.isEmpty()) {
            const memcached_return_t rt = memcached_set_sasl_auth_data(
                new_memc, saslUser.toUtf8().constData(), saslPass.toUtf8().constData());
            if (Q_LIKELY(memcached_success(rt))) {
                qCInfo(C_MEMCACHED) << "SASL authentication: enabled";
                d->saslEnabled = true;
            } else {
                qCWarning(C_MEMCACHED)
                    << "Failed to enable SASL authentication:" << memcached_strerror(new_memc, rt);
            }
        } else {
            qCInfo(C_MEMCACHED) << "SASL authentication: disabled";
        }
#    endif
#endif

        if (d->memc) {
            memcached_free(d->memc);
        }
        d->memc = new_memc;
        ok      = true;
    }

    if (ok) {
        connect(app, &Application::postForked, this, [this] { mcd = this; });
        app->loadTranslations(u"plugin_memcached"_qs);
    } else {
        qCCritical(C_MEMCACHED) << "Failed to configure the connection to the memcached server(s)";
    }

    return ok;
}

bool Memcached::set(QByteArrayView key,
                    const QByteArray &value,
                    time_t expiration,
                    ReturnType *returnType)
{
    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return false;
    }

    MemcachedPrivate::Flags flags;
    const QByteArray _value = MemcachedPrivate::compressIfNeeded(value, flags);

    const memcached_return_t rt = memcached_set(mcd->d_ptr->memc,
                                                key.constData(),
                                                key.size(),
                                                _value.constData(),
                                                _value.size(),
                                                expiration,
                                                flags);

    const bool ok = memcached_success(rt);

    if (!ok) {
        qCWarning(C_MEMCACHED).nospace()
            << "Failed to store key " << key << ": " << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::setByKey(QByteArrayView groupKey,
                         QByteArrayView key,
                         const QByteArray &value,
                         time_t expiration,
                         ReturnType *returnType)
{
    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return false;
    }

    MemcachedPrivate::Flags flags;
    const QByteArray _value = MemcachedPrivate::compressIfNeeded(value, flags);

    const memcached_return_t rt = memcached_set_by_key(mcd->d_ptr->memc,
                                                       groupKey.constData(),
                                                       groupKey.size(),
                                                       key.constData(),
                                                       key.size(),
                                                       _value.constData(),
                                                       _value.size(),
                                                       expiration,
                                                       flags);

    const bool ok = memcached_success(rt);

    if (!ok) {
        qCWarning(C_MEMCACHED).nospace()
            << "Failed to store key " << key << " on group " << groupKey << ": "
            << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::add(QByteArrayView key,
                    const QByteArray &value,
                    time_t expiration,
                    ReturnType *returnType)
{
    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return false;
    }

    MemcachedPrivate::Flags flags;
    const QByteArray _value = MemcachedPrivate::compressIfNeeded(value, flags);

    const memcached_return_t rt = memcached_add(mcd->d_ptr->memc,
                                                key.constData(),
                                                key.size(),
                                                _value.constData(),
                                                _value.size(),
                                                expiration,
                                                flags);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTSTORED)) {
        qCWarning(C_MEMCACHED).nospace()
            << "Failed to add key " << key << ": " << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::addByKey(QByteArrayView groupKey,
                         QByteArrayView key,
                         const QByteArray &value,
                         time_t expiration,
                         ReturnType *returnType)
{
    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return false;
    }

    MemcachedPrivate::Flags flags;
    const QByteArray _value = MemcachedPrivate::compressIfNeeded(value, flags);

    const memcached_return_t rt = memcached_add_by_key(mcd->d_ptr->memc,
                                                       groupKey.constData(),
                                                       groupKey.size(),
                                                       key.constData(),
                                                       key.size(),
                                                       _value.constData(),
                                                       _value.size(),
                                                       expiration,
                                                       flags);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTSTORED)) {
        qCWarning(C_MEMCACHED).nospace() << "Failed to add key " << key << " on group " << groupKey
                                         << ": " << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::replace(QByteArrayView key,
                        const QByteArray &value,
                        time_t expiration,
                        ReturnType *returnType)
{
    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return false;
    }

    MemcachedPrivate::Flags flags;
    const QByteArray _value = MemcachedPrivate::compressIfNeeded(value, flags);

    const memcached_return_t rt = memcached_replace(mcd->d_ptr->memc,
                                                    key.constData(),
                                                    key.size(),
                                                    _value.constData(),
                                                    _value.size(),
                                                    expiration,
                                                    flags);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTSTORED)) {
        qCWarning(C_MEMCACHED).nospace()
            << "Failed to replace key " << key << ": " << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::replaceByKey(QByteArrayView groupKey,
                             QByteArrayView key,
                             const QByteArray &value,
                             time_t expiration,
                             ReturnType *returnType)
{
    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return false;
    }

    MemcachedPrivate::Flags flags;
    const QByteArray _value = MemcachedPrivate::compressIfNeeded(value, flags);

    const memcached_return_t rt = memcached_replace_by_key(mcd->d_ptr->memc,
                                                           groupKey.constData(),
                                                           groupKey.size(),
                                                           key.constData(),
                                                           key.size(),
                                                           _value.constData(),
                                                           _value.size(),
                                                           expiration,
                                                           flags);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTSTORED)) {
        qCWarning(C_MEMCACHED).nospace()
            << "Failed to replace key " << key << " on group " << groupKey << ": "
            << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

QByteArray Memcached::get(QByteArrayView key, uint64_t *cas, ReturnType *returnType)
{
    QByteArray retData;

    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return retData;
    }

    memcached_return_t rt{MEMCACHED_FAILURE};
    bool ok = false;

    std::vector<const char *> keys;
    std::vector<size_t> sizes;
    keys.push_back(key.constData());
    sizes.push_back(key.size());
    rt = memcached_mget(mcd->d_ptr->memc, &keys[0], &sizes[0], keys.size());

    if (memcached_success(rt)) {
        memcached_result_st *result = memcached_fetch_result(mcd->d_ptr->memc, nullptr, &rt);
        if (result) {
            retData =
                QByteArray(memcached_result_value(result),
                           static_cast<QByteArray::size_type>(memcached_result_length(result)));
            if (cas) {
                *cas = memcached_result_cas(result);
            }
            retData = MemcachedPrivate::uncompressIfNeeded(retData, result);
            ok      = true;
            // fetch another result even if there is no one to get
            // a NULL for the internal of libmemcached
            memcached_fetch_result(mcd->d_ptr->memc, nullptr, nullptr);
        }
        memcached_result_free(result);
    }

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED).nospace() << "Failed to get data for key " << key << ": "
                                         << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return retData;
}

QByteArray Memcached::getByKey(QByteArrayView groupKey,
                               QByteArrayView key,
                               uint64_t *cas,
                               ReturnType *returnType)
{
    QByteArray retData;

    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return retData;
    }

    bool ok = false;

    std::vector<const char *> keys;
    std::vector<size_t> sizes;
    keys.push_back(key.constData());
    sizes.push_back(key.size());
    memcached_return_t rt = memcached_mget_by_key(
        mcd->d_ptr->memc, groupKey.constData(), groupKey.size(), &keys[0], &sizes[0], keys.size());

    if (memcached_success(rt)) {
        memcached_result_st *result = memcached_fetch_result(mcd->d_ptr->memc, nullptr, &rt);
        if (result) {
            retData =
                QByteArray(memcached_result_value(result),
                           static_cast<QByteArray::size_type>(memcached_result_length(result)));
            if (cas) {
                *cas = memcached_result_cas(result);
            }
            retData = MemcachedPrivate::uncompressIfNeeded(retData, result);
            ok      = true;
            // fetch another result even if there is no one to get
            // a NULL for the internal of libmemcached
            memcached_fetch_result(mcd->d_ptr->memc, nullptr, nullptr);
        }
        memcached_result_free(result);
    }

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED).nospace()
            << "Failed to get data for key " << key << " on group " << groupKey << ": "
            << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return retData;
}

bool Memcached::remove(QByteArrayView key, ReturnType *returnType)
{
    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return false;
    }

    const memcached_return_t rt =
        memcached_delete(mcd->d_ptr->memc, key.constData(), key.size(), 0);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED).nospace() << "Failed to remove data for key " << key << ": "
                                         << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::removeByKey(QByteArrayView groupKey, QByteArrayView key, ReturnType *returnType)
{
    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return false;
    }

    const memcached_return_t rt = memcached_delete_by_key(
        mcd->d_ptr->memc, groupKey.constData(), groupKey.size(), key.constData(), key.size(), 0);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED).nospace()
            << "Failed to remove data for key " << key << " on group " << groupKey << ": "
            << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::exist(QByteArrayView key, ReturnType *returnType)
{
    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return false;
    }

    const memcached_return_t rt = memcached_exist(mcd->d_ptr->memc, key.constData(), key.size());

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED).nospace() << "Failed to check existence of key " << key << ": "
                                         << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::existByKey(QByteArrayView groupKey, QByteArrayView key, ReturnType *returnType)
{
    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return false;
    }

    const memcached_return_t rt = memcached_exist_by_key(
        mcd->d_ptr->memc, groupKey.constData(), groupKey.size(), key.constData(), key.size());

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED).nospace()
            << "Failed to check existence of key " << key << " in group " << groupKey << ": "
            << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::increment(QByteArrayView key,
                          uint32_t offset,
                          uint64_t *value,
                          ReturnType *returnType)
{
    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return false;
    }

    const memcached_return_t rt =
        memcached_increment(mcd->d_ptr->memc, key.constData(), key.size(), offset, value);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED).nospace() << "Failed to increment key " << key << " by " << offset
                                         << ": " << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::incrementByKey(QByteArrayView groupKey,
                               QByteArrayView key,
                               uint64_t offset,
                               uint64_t *value,
                               ReturnType *returnType)
{
    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return false;
    }

    const memcached_return_t rt = memcached_increment_by_key(mcd->d_ptr->memc,
                                                             groupKey.constData(),
                                                             groupKey.size(),
                                                             key.constData(),
                                                             key.size(),
                                                             offset,
                                                             value);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED).nospace()
            << "Failed to increment key " << key << " in group " << groupKey << " by " << offset
            << ": " << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::incrementWithInitial(QByteArrayView key,
                                     uint64_t offset,
                                     uint64_t initial,
                                     time_t expiration,
                                     uint64_t *value,
                                     ReturnType *returnType)
{
    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return false;
    }

    const memcached_return_t rt = memcached_increment_with_initial(
        mcd->d_ptr->memc, key.constData(), key.size(), offset, initial, expiration, value);

    const bool ok = memcached_success(rt);

    if (!ok) {
        qCWarning(C_MEMCACHED).nospace()
            << "Failed to increment or initialize key " << key << " by offset " << offset
            << " or initial " << initial << ": " << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::incrementWithInitialByKey(QByteArrayView groupKey,
                                          QByteArrayView key,
                                          uint64_t offset,
                                          uint64_t initial,
                                          time_t expiration,
                                          uint64_t *value,
                                          ReturnType *returnType)
{
    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return false;
    }

    const memcached_return_t rt = memcached_increment_with_initial_by_key(mcd->d_ptr->memc,
                                                                          groupKey.constData(),
                                                                          groupKey.size(),
                                                                          key.constData(),
                                                                          key.size(),
                                                                          offset,
                                                                          initial,
                                                                          expiration,
                                                                          value);

    const bool ok = memcached_success(rt);
    if (!ok) {
        qCWarning(C_MEMCACHED).nospace()
            << "Failed to increment or initializes key " << key << " in group " << groupKey
            << " by offset " << offset << " or initial " << initial << ": "
            << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::decrement(QByteArrayView key,
                          uint32_t offset,
                          uint64_t *value,
                          ReturnType *returnType)
{
    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return false;
    }

    const memcached_return_t rt =
        memcached_decrement(mcd->d_ptr->memc, key.constData(), key.size(), offset, value);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED).nospace() << "Failed to decrement key " << key << " by " << offset
                                         << ": " << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::decrementByKey(QByteArrayView groupKey,
                               QByteArrayView key,
                               uint64_t offset,
                               uint64_t *value,
                               ReturnType *returnType)
{
    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return false;
    }

    const memcached_return_t rt = memcached_decrement_by_key(mcd->d_ptr->memc,
                                                             groupKey.constData(),
                                                             groupKey.size(),
                                                             key.constData(),
                                                             key.size(),
                                                             offset,
                                                             value);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED).nospace()
            << "Failed to decrement key " << key << " in group " << groupKey << " by " << offset
            << ": " << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::decrementWithInitial(QByteArrayView key,
                                     uint64_t offset,
                                     uint64_t initial,
                                     time_t expiration,
                                     uint64_t *value,
                                     ReturnType *returnType)
{
    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return false;
    }

    const memcached_return_t rt = memcached_decrement_with_initial(
        mcd->d_ptr->memc, key.constData(), key.size(), offset, initial, expiration, value);

    const bool ok = memcached_success(rt);

    if (!ok) {
        qCWarning(C_MEMCACHED).nospace()
            << "Failed to decrement of initialize key " << key << " by offset " << offset
            << " or initialize " << initial << ": " << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::decrementWithInitialByKey(QByteArrayView groupKey,
                                          QByteArrayView key,
                                          uint64_t offset,
                                          uint64_t initial,
                                          time_t expiration,
                                          uint64_t *value,
                                          ReturnType *returnType)
{
    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return false;
    }

    const memcached_return_t rt = memcached_decrement_with_initial_by_key(mcd->d_ptr->memc,
                                                                          groupKey.constData(),
                                                                          groupKey.size(),
                                                                          key.constData(),
                                                                          key.size(),
                                                                          offset,
                                                                          initial,
                                                                          expiration,
                                                                          value);

    const bool ok = memcached_success(rt);
    if (!ok) {
        qCWarning(C_MEMCACHED).nospace()
            << "Failed to decrement or initialize key " << key << " in group " << groupKey
            << " by offset " << offset << " or initial " << initial << ": "
            << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::cas(QByteArrayView key,
                    const QByteArray &value,
                    time_t expiration,
                    uint64_t cas,
                    ReturnType *returnType)
{
    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return false;
    }

    MemcachedPrivate::Flags flags;
    const QByteArray _value = MemcachedPrivate::compressIfNeeded(value, flags);

    const memcached_return_t rt = memcached_cas(mcd->d_ptr->memc,
                                                key.constData(),
                                                key.size(),
                                                _value.constData(),
                                                _value.size(),
                                                expiration,
                                                flags,
                                                cas);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_DATA_EXISTS)) {
        qCWarning(C_MEMCACHED).nospace() << "Failed to compare and set (cas) key " << key << ": "
                                         << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::casByKey(QByteArrayView groupKey,
                         QByteArrayView key,
                         const QByteArray &value,
                         time_t expiration,
                         uint64_t cas,
                         ReturnType *returnType)
{
    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return false;
    }

    MemcachedPrivate::Flags flags;
    const QByteArray _value = MemcachedPrivate::compressIfNeeded(value, flags);

    const memcached_return_t rt = memcached_cas_by_key(mcd->d_ptr->memc,
                                                       groupKey.constData(),
                                                       groupKey.size(),
                                                       key.constData(),
                                                       key.size(),
                                                       _value.constData(),
                                                       _value.size(),
                                                       expiration,
                                                       flags,
                                                       cas);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_DATA_EXISTS)) {
        qCWarning(C_MEMCACHED).nospace()
            << "Failed to compare and set (cas) key " << key << " in group " << groupKey << ": "
            << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::flushBuffers(ReturnType *returnType)
{
    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return false;
    }

    const memcached_return_t rt = memcached_flush_buffers(mcd->d_ptr->memc);

    const bool ok = memcached_success(rt);

    if (!ok) {
        qCWarning(C_MEMCACHED) << "Failed to flush buffers:"
                               << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::flush(time_t expiration, ReturnType *returnType)
{
    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return false;
    }

    const memcached_return_t rt = memcached_flush(mcd->d_ptr->memc, expiration);

    const bool ok = memcached_success(rt);

    if (!ok) {
        qCWarning(C_MEMCACHED) << "Failed to wipe (flush) server content:"
                               << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

QHash<QByteArray, QByteArray> Memcached::mget(const QByteArrayList &keys,
                                              QHash<QByteArray, uint64_t> *casValues,
                                              ReturnType *returnType)
{
    QHash<QByteArray, QByteArray> ret;

    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return ret;
    }

    if (keys.empty()) {
        qCWarning(C_MEMCACHED) << "Can not get multiple values without a list of keys.";
        if (returnType) {
            *returnType = ReturnType::BadKeyProvided;
        }
        return ret;
    }

    std::vector<const char *> _keys;
    _keys.reserve(keys.size());
    std::vector<size_t> _keysSizes;
    _keysSizes.reserve(keys.size());

    for (const auto &key : keys) {
        _keys.push_back(key.data());
        _keysSizes.push_back(key.size());
    }

    bool ok = false;

    memcached_return_t rt =
        memcached_mget(mcd->d_ptr->memc, &_keys[0], &_keysSizes[0], _keys.size());

    if (memcached_success(rt)) {
        ok = true;
        ret.reserve(keys.size());
        while ((rt != MEMCACHED_END) && (rt != MEMCACHED_NOTFOUND)) {
            memcached_result_st *result = memcached_fetch_result(mcd->d_ptr->memc, nullptr, &rt);
            if (result) {
                const QByteArray rk =
                    QByteArray(memcached_result_key_value(result),
                               static_cast<qsizetype>(memcached_result_key_length(result)));
                QByteArray rd(memcached_result_value(result),
                              static_cast<qsizetype>(memcached_result_length(result)));
                if (casValues) {
                    casValues->insert(rk, memcached_result_cas(result));
                }
                rd = MemcachedPrivate::uncompressIfNeeded(rd, result);
                ret.insert(rk, rd);
            }
            memcached_result_free(result);
        }
    }

    if (!ok) {
        qCWarning(C_MEMCACHED) << "Failed to get values for multiple keys:"
                               << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ret;
}

QHash<QByteArray, QByteArray> Memcached::mgetByKey(QByteArrayView groupKey,
                                                   const QByteArrayList &keys,
                                                   QHash<QByteArray, uint64_t> *casValues,
                                                   ReturnType *returnType)
{
    QHash<QByteArray, QByteArray> ret;

    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return ret;
    }

    if (groupKey.isEmpty()) {
        qCWarning(C_MEMCACHED)
            << "Can not get multiple values from specific server when groupKey is empty.";
        if (returnType) {
            *returnType = ReturnType::BadKeyProvided;
        }
        return ret;
    }

    if (keys.empty()) {
        qCWarning(C_MEMCACHED) << "Can not get multiple values without a list of keys.";
        if (returnType) {
            *returnType = ReturnType::BadKeyProvided;
        }
        return ret;
    }

    std::vector<const char *> _keys;
    _keys.reserve(keys.size());
    std::vector<size_t> _keysSizes;
    _keysSizes.reserve(keys.size());

    for (const auto &key : keys) {
        _keys.push_back(key.data());
        _keysSizes.push_back(key.size());
    }

    bool ok = false;

    memcached_return_t rt = memcached_mget_by_key(mcd->d_ptr->memc,
                                                  groupKey.constData(),
                                                  groupKey.size(),
                                                  &_keys[0],
                                                  &_keysSizes[0],
                                                  _keys.size());

    if (memcached_success(rt)) {
        ok = true;
        ret.reserve(keys.size());
        while ((rt != MEMCACHED_END) && (rt != MEMCACHED_NOTFOUND)) {
            memcached_result_st *result = memcached_fetch_result(mcd->d_ptr->memc, nullptr, &rt);
            if (result) {
                const QByteArray rk =
                    QByteArray(memcached_result_key_value(result),
                               static_cast<qsizetype>(memcached_result_key_length(result)));
                QByteArray rd(memcached_result_value(result),
                              static_cast<qsizetype>(memcached_result_length(result)));
                if (casValues) {
                    casValues->insert(rk, memcached_result_cas(result));
                }
                rd = MemcachedPrivate::uncompressIfNeeded(rd, result);
                ret.insert(rk, rd);
            }
            memcached_result_free(result);
        }
    }

    if (!ok) {
        qCWarning(C_MEMCACHED).nospace()
            << "Failed to get values for multiple keys in group " << groupKey << ": "
            << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ret;
}

bool Memcached::touch(QByteArrayView key, time_t expiration, ReturnType *returnType)
{
    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return false;
    }

    const memcached_return_t rt =
        memcached_touch(mcd->d_ptr->memc, key.constData(), key.size(), expiration);

    const bool ok = memcached_success(rt);

    if (!ok) {
        qCWarning(C_MEMCACHED).nospace()
            << "Failed to touch key " << key << " with new expiration time " << expiration
            << " seconds: " << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::touchByKey(QByteArrayView groupKey,
                           QByteArrayView key,
                           time_t expiration,
                           ReturnType *returnType)
{
    if (!MemcachedPrivate::isRegistered(mcd, returnType)) {
        return false;
    }

    const memcached_return_t rt = memcached_touch_by_key(mcd->d_ptr->memc,
                                                         groupKey.constData(),
                                                         groupKey.size(),
                                                         key.constData(),
                                                         key.size(),
                                                         expiration);

    const bool ok = memcached_success(rt);

    if (!ok) {
        qCWarning(C_MEMCACHED).nospace()
            << "Failed to touch key " << key << " in group " << groupKey
            << " with new expiration time " << expiration
            << " seconds: " << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

QString Memcached::errorString(Context *c, ReturnType rt)
{
    switch (rt) {
        using enum Memcached::ReturnType;
    case Success:
        //% "The request was successfully executed."
        return c->qtTrId("cutelyst-memc-rt-success");
    case Failure:
        //% "An unknown failure has occurred in the Memcached server."
        return c->qtTrId("cutelyst-memc-rt-failure");
    case HostLookupFailure:
        //% "Failed to look up the hostname while trying to connect to a Memcached server."
        return c->qtTrId("cutelyst-memc-rt-hostlookupfailure");
    case ConnectionFailure:
        //% "An unknown error has occurred while trying to connect to a Memcached server."
        return c->qtTrId("cutelyst-memc-rt-connectionfailure");
    case WriteFailure:
        //% "An error has occurred while trying to write to a Memcached server."
        return c->qtTrId("cutelyst-memc-rt-writefailure");
    case ReadFailure:
        //% "An error has occurred while trying to read from a Memcached server."
        return c->qtTrId("cutelyst-memc-rt-readfailure");
    case UnknownReadFailure:
        //% "An unknown error has occurred while trying to read from a Memcached server. This "
        //% "only occures when either there is a bug in the server, or in rare cases where an "
        //% "ethernet NIC is reporting dubious information."
        return c->qtTrId("cutelyst-memc-rt-unknownreadfailure");
    case ProtocolError:
        //% "An unknown error has occurred in the Memcached protocol."
        return c->qtTrId("cutelyst-memc-rt-protocolerror");
    case ClientError:
        //% "An unknown Memcached client error has occurred internally."
        return c->qtTrId("cutelyst-memc-rt-clienterror");
    case ServerError:
        //% "An unknown error has occurred in the Memcached server."
        return c->qtTrId("cutelyst-memc-rt-servererror");
    case Error:
        //% "A general error occurred."
        return c->qtTrId("cutelyst-memc-rt-error");
    case DataExists:
        //% "The data for the given key alrey exists."
        return c->qtTrId("cutelyst-memc-rt-dataexists");
    case DataDoesNotExist:
        //% "The data requested with the key given was not found."
        return c->qtTrId("cutelyst-memc-rt-datadoesnotexist");
    case NotStored:
        //% "The request to store an object failed."
        return c->qtTrId("cutelyst-memc-rt-notstored");
    case Stored:
        //% "The requested object has been successfully stored on the server."
        return c->qtTrId("cutelyst-memc-rt-stored");
    case NotFound:
        //% "The object requested was not found."
        return c->qtTrId("cutelyst-memc-notfound");
    case MemoryAllocationFailure:
        //% "An error has occurred while trying to allocate memory."
        return c->qtTrId("cutelyst-memc-rt-memallocfailure");
    case PartialRead:
        //% "The read operation was only partcially successful."
        return c->qtTrId("cutelyst-memc-rt-partread");
    case SomeErrors:
        //% "A multi request has been made, and some underterminate number of "
        //% "errors have occurred."
        return c->qtTrId("cutelyst-memc-rt-someerrors");
    case NoServers:
        //% "No servers have been added to the Memcached plugin."
        return c->qtTrId("cutelyst-memc-rt-noservers");
    case End:
        //% "The Memcached server has completed returning all of the objects requested."
        return c->qtTrId("cutelyst-memc-rt-end");
    case Deleted:
        //% "The object requested by the key has been deleted."
        return c->qtTrId("cutelyst-memc-rt-deleted");
    case Stat:
        //% "A “stat” command has been returned in the protocol."
        return c->qtTrId("cutelyst-memc-rt-stat");
    case Errno:
        //% "An error has occurred in the driver which has set errno."
        return qtTrId("cutelyst-memc-rt-errno");
    case NotSupported:
        //% "The given method is not supported in the Memcached server."
        return c->qtTrId("cutelyst-memc-rt-notsupported");
    case FetchNotFinished:
        //% "A request has been made, but the Memcached server has not finished "
        //% "the fetch of the last request."
        return c->qtTrId("cutelyst-memc-rt-fetchnotfinished");
    case Timeout:
        //% "The operation has timed out."
        return c->qtTrId("cutelyst-memc-rt-timeout");
    case Buffered:
        //% "The request has been buffered."
        return c->qtTrId("cutelyst-memc-rt-buffered");
    case BadKeyProvided:
        //% "The key provided is not a valid key."
        return c->qtTrId("cutelyst-memc-rt-badkeyprov");
    case InvalidHostProtocol:
        //% "The Memcached server you are connecting to has an invalid protocol. Most likely you "
        //% "are connecting to an older server that does not speak the binary protocol."
        return c->qtTrId("cutelyst-memc-rt-invalidhostprot");
    case ServerMarkedDead:
        //% "The requested Memcached server has been marked dead."
        return c->qtTrId("cutelyst-memc-rt-servermarkeddead");
    case UnknownStatKey:
        //% "The Memcached server you are communicating with has a stat key which "
        //% "has not be defined in the protocol."
        return c->qtTrId("cutelyst-memc-rt-unknownstatkey");
    case E2Big:
        //% "Item is too large for the Memcached server to store."
        return c->qtTrId("cutelyst-memc-rt-e2big");
    case InvalidArguments:
        //% "The arguments supplied to the given function were not valid."
        return c->qtTrId("cutelyst-memc-rt-invalidarg");
    case KeyTooBig:
        //% "The key that has been provided is too large for the given Memcached server."
        return c->qtTrId("cutelyst-memc-rt-key2big");
    case AuthProblem:
        //% "An unknown issue has occurred during SASL authentication."
        return c->qtTrId("cutelyst-memc-rt-authproblem");
    case AuthFailure:
        //% "The credentials provided are not valid for this Memcached server."
        return c->qtTrId("cutelyst-memc-rt-authfailure");
    case AuthContinue:
        //% "Authentication has been paused."
        return c->qtTrId("cutelyst-memc-rt-authcont");
    case ParseError:
        //% "An error has occurred while trying to parse the configuration string."
        return c->qtTrId("cutelyst-memc-rt-parseerr");
    case ParseUserError:
        //% "An error has occurred in parsing the configuration string."
        return c->qtTrId("cutelyst-memc-rt-parseusererr");
    case Deprecated:
        //% "The method that was requested has been deprecated."
        return c->qtTrId("cutelyst-memc-rt-deprecated");
    case PluginNotRegisterd:
        //% "The Cutelyst Memcached plugin has not been registered to the application."
        return c->qtTrId("cutelyst-memc-rt-pluginnotregistered");
    default:
        //% "An unknown error has occurred."
        return c->qtTrId("cutelyst-memc-rt-unknown-err");
    }
}

QVersionNumber Memcached::libMemcachedVersion()
{
    return QVersionNumber::fromString(QLatin1String(memcached_lib_version()));
}

// clang-format off
/**
 * @internal
 * Converts the @a rt returned by libmemcached into a ReturnType enum.
 */
Memcached::ReturnType MemcachedPrivate::returnTypeConvert(memcached_return_t rt)
{
    switch (rt) {
    using enum Memcached::ReturnType;
    case MEMCACHED_SUCCESS:                             return Success;
    case MEMCACHED_FAILURE:                             return Failure;
    case MEMCACHED_HOST_LOOKUP_FAILURE:                 return HostLookupFailure;
    case MEMCACHED_CONNECTION_FAILURE:                  return ConnectionFailure;
    case MEMCACHED_CONNECTION_BIND_FAILURE:             return HostLookupFailure;
    case MEMCACHED_WRITE_FAILURE:                       return WriteFailure;
    case MEMCACHED_READ_FAILURE:                        return ReadFailure;
    case MEMCACHED_UNKNOWN_READ_FAILURE:                return UnknownReadFailure;
    case MEMCACHED_PROTOCOL_ERROR:                      return ProtocolError;
    case MEMCACHED_CLIENT_ERROR:                        return ClientError;
    case MEMCACHED_SERVER_ERROR:                        return ServerError;
    case MEMCACHED_ERROR:                               return Error;
    case MEMCACHED_DATA_EXISTS:                         return DataExists;
    case MEMCACHED_DATA_DOES_NOT_EXIST:                 return DataDoesNotExist;
    case MEMCACHED_NOTSTORED:                           return NotStored;
    case MEMCACHED_STORED:                              return Stored;
    case MEMCACHED_NOTFOUND:                            return NotFound;
    case MEMCACHED_MEMORY_ALLOCATION_FAILURE:           return MemoryAllocationFailure;
    case MEMCACHED_PARTIAL_READ:                        return PartialRead;
    case MEMCACHED_SOME_ERRORS:                         return SomeErrors;
    case MEMCACHED_NO_SERVERS:                          return NoServers;
    case MEMCACHED_END:                                 return End;
    case MEMCACHED_DELETED:                             return Deleted;
    case MEMCACHED_STAT:                                return Stat;
    case MEMCACHED_ERRNO:                               return Errno;
    case MEMCACHED_NOT_SUPPORTED:                       return NotSupported;
    case MEMCACHED_FETCH_NOTFINISHED:                   return FetchNotFinished;
    case MEMCACHED_TIMEOUT:                             return Timeout;
    case MEMCACHED_BUFFERED:                            return Buffered;
    case MEMCACHED_BAD_KEY_PROVIDED:                    return BadKeyProvided;
    case MEMCACHED_INVALID_HOST_PROTOCOL:               return InvalidHostProtocol;
    case MEMCACHED_SERVER_MARKED_DEAD:                  return ServerMarkedDead;
    case MEMCACHED_UNKNOWN_STAT_KEY:                    return UnknownStatKey;
    case MEMCACHED_E2BIG:                               return E2Big;
    case MEMCACHED_INVALID_ARGUMENTS:                   return InvalidArguments;
    case MEMCACHED_KEY_TOO_BIG:                         return KeyTooBig;
    case MEMCACHED_AUTH_PROBLEM:                        return AuthProblem;
    case MEMCACHED_AUTH_FAILURE:                        return AuthFailure;
    case MEMCACHED_AUTH_CONTINUE:                       return AuthContinue;
    case MEMCACHED_PARSE_ERROR:                         return ParseError;
    case MEMCACHED_PARSE_USER_ERROR:                    return ParseUserError;
    case MEMCACHED_DEPRECATED:                          return Deprecated;
    case MEMCACHED_IN_PROGRESS:                         return InProgress;
    case MEMCACHED_SERVER_TEMPORARILY_DISABLED:         return ServerTemporaryDisabled;
    case MEMCACHED_SERVER_MEMORY_ALLOCATION_FAILURE:    return ServerMemoryAllocationFailure;
    case MEMCACHED_MAXIMUM_RETURN:                      return MaximumReturn;
    default:                                            return Success;
    }
}
// clang-format on

/**
 * @internal
 * Convenience function to set @a rt2 returned by libmemcached to a pointer to
 * a ReturnType at @a rt1 if @a rt1 is not a @c nullptr.
 */
void MemcachedPrivate::setReturnType(Memcached::ReturnType *rt1, memcached_return_t rt2)
{
    if (rt1) {
        *rt1 = MemcachedPrivate::returnTypeConvert(rt2);
    }
}

/**
 * @internal
 * Checks if the %Memcached plugin @a ptr is registered to the application and sets
 * the @a rt accordingly.
 */
bool MemcachedPrivate::isRegistered(Memcached *ptr, Memcached::ReturnType *rt)
{
    if (!ptr) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (rt) {
            *rt = Memcached::ReturnType::PluginNotRegisterd;
        }
        return false;
    }
    return true;
}

/**
 * @internal
 * Compresses the @a value with @link QByteArray::qCompress() qCompress()@endlink if compression
 * has been enabled. Will also set the correct @a flags.
 */
QByteArray MemcachedPrivate::compressIfNeeded(const QByteArray &value, Flags &flags)
{
    if (mcd->d_ptr->compression && (value.size() > mcd->d_ptr->compressionThreshold)) {
        flags |= MemcachedPrivate::Compressed;
        return qCompress(value, mcd->d_ptr->compressionLevel);
    } else {
        return value;
    }
}

/**
 * @internal
 * Reads the stored flags from @a result and uncompresses the @a value if needed
 * with @link QByteArray::qUncompress() qUncompress()@endlink and returns it.
 */
QByteArray MemcachedPrivate::uncompressIfNeeded(const QByteArray &value,
                                                memcached_result_st *result)
{
    const MemcachedPrivate::Flags flags{memcached_result_flags(result)};
    if (flags.testFlag(MemcachedPrivate::Compressed)) {
        return qUncompress(value);
    } else {
        return value;
    }
}

/**
 * @internal
 * Returns the value stored for @a key from the configuration. First tries to read the
 * value from the configuration loaded from the config file. If there is no such @a key it
 * tries to find it in the default configuration. If the @a key is not available there, too,
 * it will return the @a defaultValue.
 */
QVariant MemcachedPrivate::config(const QString &key, const QVariant &defaultValue) const
{
    return loadedConfig.value(key, defaultConfig.value(key, defaultValue));
}

#include "moc_memcached.cpp"
