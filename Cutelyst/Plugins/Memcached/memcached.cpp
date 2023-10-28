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

static thread_local Memcached *mcd       = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
const time_t Memcached::expirationNotAdd = MEMCACHED_EXPIRATION_NOT_ADD;

Memcached::Memcached(Application *parent)
    : Plugin(parent)
    , d_ptr(new MemcachedPrivate)
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

    const QVariantMap map = app->engine()->config(u"Cutelyst_Memcached_Plugin"_qs);
    QStringList config;

    const QStringList serverList =
        map.value(u"servers"_qs, d->defaultConfig.value(u"servers"_qs))
            .toString()
            .split(u';');

    if (serverList.empty()) {
        config.push_back(u"--SERVER=localhost"_qs);
    }

    for (const QString &flag : {
             u"verify_key"_qs,
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
             u"tcp_keepalive"_qs
         }) {
        if (map.value(flag, d->defaultConfig.value(flag, false)).toBool()) {
            const QString flagStr = u"--" + flag.toUpper().replace(u'_', u'-');
            config.push_back(flagStr);
        }
    }

    const bool useUDP = map.value(u"use_udp"_qs,
                                  d->defaultConfig.value(u"use_udp"_qs, false))
                            .toBool();

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
        const QString _val = map.value(opt, d->defaultConfig.value(opt)).toString();
        if (!_val.isEmpty()) {
            const QString optStr = u"--" + opt.toUpper().replace(u'_', u'-') + u'=' + _val;
            config.push_back(optStr); // clazy:exclude=reserve-candidates
        }
    }

    const QByteArray configString = config.join(u' ').toUtf8();

    bool ok = false;

    qCInfo(C_MEMCACHED) << "Setting up connection to memcached servers using libmemcached" << memcached_lib_version() << "with the following configuration string:" << configString;

    memcached_st *new_memc = memcached(configString.constData(), configString.size());

    if (new_memc) {

        if (!serverList.empty()) {
            for (const QString &server : serverList) {
                const auto serverParts = QStringView(server).split(u',');
                QString name;
                uint port       = 11211;
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
                    memcached_return_t rc;
                    if (isSocket) {
                        rc = memcached_server_add_unix_socket_with_weight(
                            new_memc, name.toUtf8().constData(), weight);
                        if (Q_LIKELY(memcached_success(rc))) {
                            qCInfo(C_MEMCACHED) << "Added memcached server on socket" << name << "with weight" << weight;
                        } else {
                            qCWarning(C_MEMCACHED).nospace() << "Failed to add memcached server on socket "
                                                             << name << " with weight " << weight << ": "
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
                            qCInfo(C_MEMCACHED).nospace().noquote() << "Added memcached server on host "
                                                                    << name << ":" << port << " with weight" << weight;
                        } else {
                            qCWarning(C_MEMCACHED).nospace().noquote() << "Failed to add memcached server no host "
                                                                       << name << ":" << port << " with weight "
                                                                       << weight << ": " << memcached_strerror(new_memc, rc);
                        }
                    }
                }
            }

            if (Q_UNLIKELY(memcached_server_count(new_memc) == 0)) {
                qCWarning(C_MEMCACHED) << "Failed to add any memcached server. Adding default server on localhost"
                                       << "port 11211.";
                memcached_return_t rc = memcached_server_add(new_memc, "localhost", 11211);
                if (Q_UNLIKELY(!memcached_success(rc))) {
                    qCCritical(C_MEMCACHED) << "Failed to add default memcached server. Memcached plugin will not"
                                            << "work without a configured server!" << memcached_strerror(new_memc, rc);
                    memcached_free(new_memc);
                    return false;
                }
            }
        }

        d->compression = map.value(u"compression"_qs,
                                   d->defaultConfig.value(u"compression"_qs, false))
                             .toBool();
        d->compressionLevel =
            map.value(u"compression_level"_qs,
                      d->defaultConfig.value(u"compression_level"_qs, -1))
                .toInt();
        d->compressionThreshold =
            map.value(u"compression_threshold"_qs,
                      d->defaultConfig.value(u"compression_threshold"_qs, 100))
                .toInt();
        if (d->compression) {
            qCInfo(C_MEMCACHED).nospace() << "Compression: enabled (Compression level: " << d->compressionLevel
                                          << ", Compression threshold: " << d->compressionThreshold << " bytes";
        } else {
            qCInfo(C_MEMCACHED) << "Compression: disabled";
        }

        const QString encKey = map.value(u"encryption_key"_qs).toString();
        if (!encKey.isEmpty()) {
            const QByteArray encKeyBa = encKey.toUtf8();
            const memcached_return_t rt =
                memcached_set_encoding_key(new_memc, encKeyBa.constData(), encKeyBa.size());
            if (Q_LIKELY(memcached_success(rt))) {
                qCInfo(C_MEMCACHED) << "Encryption: enabled";
            } else {
                qCWarning(C_MEMCACHED) << "Failed to enable encryption:"
                                       << memcached_strerror(new_memc, rt);
            }
        } else {
            qCInfo(C_MEMCACHED) << "Encryption: disabled";
        }

#ifdef LIBMEMCACHED_WITH_SASL_SUPPORT
#    if LIBMEMCACHED_WITH_SASL_SUPPORT == 1
        const QString saslUser = map.value(u"sasl_user"_qs).toString();
        const QString saslPass = map.value(u"sasl_password"_qs).toString();
        if (!saslUser.isEmpty() && !saslPass.isEmpty()) {
            const memcached_return_t rt = memcached_set_sasl_auth_data(
                new_memc, saslUser.toUtf8().constData(), saslPass.toUtf8().constData());
            if (Q_LIKELY(memcached_success(rt))) {
                qCInfo(C_MEMCACHED) << "SASL authentication: enabled";
                d->saslEnabled = true;
            } else {
                qCWarning(C_MEMCACHED) << "Failed to enable SASL authentication:"
                                       << memcached_strerror(new_memc, rt);
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
                    Cutelyst::Memcached::MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    MemcachedPrivate::Flags flags;
    QByteArray _value = value;

    if (mcd->d_ptr->compression && (_value.size() > mcd->d_ptr->compressionThreshold)) {
        flags |= MemcachedPrivate::Compressed;
        _value = qCompress(value, mcd->d_ptr->compressionLevel);
    }

    const memcached_return_t rt = memcached_set(mcd->d_ptr->memc,
                                                key.constData(),
                                                key.size(),
                                                _value.constData(),
                                                _value.size(),
                                                expiration,
                                                flags);

    const bool ok = memcached_success(rt);

    if (!ok) {
        qCWarning(C_MEMCACHED).nospace() << "Failed to store key " << key << ": "
                                         << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::setByKey(QByteArrayView groupKey,
                         QByteArrayView key,
                         const QByteArray &value,
                         time_t expiration,
                         MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    MemcachedPrivate::Flags flags;
    QByteArray _value = value;

    if (mcd->d_ptr->compression && (_value.size() > mcd->d_ptr->compressionThreshold)) {
        flags |= MemcachedPrivate::Compressed;
        _value = qCompress(value, mcd->d_ptr->compressionLevel);
    }

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
        qCWarning(C_MEMCACHED).nospace() << "Failed to store key " << key
                                         << " on group " << groupKey << ": "
                                         << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::add(QByteArrayView key,
                    const QByteArray &value,
                    time_t expiration,
                    MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    MemcachedPrivate::Flags flags;
    QByteArray _value = value;

    if (mcd->d_ptr->compression && (_value.size() > mcd->d_ptr->compressionThreshold)) {
        flags |= MemcachedPrivate::Compressed;
        _value = qCompress(value, mcd->d_ptr->compressionLevel);
    }

    const memcached_return_t rt = memcached_add(mcd->d_ptr->memc,
                                                key.constData(),
                                                key.size(),
                                                _value.constData(),
                                                _value.size(),
                                                expiration,
                                                flags);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTSTORED)) {
        qCWarning(C_MEMCACHED).nospace() << "Failed to add key " << key
                                         << ": " << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::addByKey(QByteArrayView groupKey,
                         QByteArrayView key,
                         const QByteArray &value,
                         time_t expiration,
                         MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    MemcachedPrivate::Flags flags;
    QByteArray _value = value;

    if (mcd->d_ptr->compression && (_value.size() > mcd->d_ptr->compressionThreshold)) {
        flags |= MemcachedPrivate::Compressed;
        _value = qCompress(value, mcd->d_ptr->compressionLevel);
    }

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
        qCWarning(C_MEMCACHED).nospace() << "Failed to add key " << key
                                         << " on group " << groupKey << ": "
                                         << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::replace(QByteArrayView key,
                        const QByteArray &value,
                        time_t expiration,
                        MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    MemcachedPrivate::Flags flags;
    QByteArray _value = value;

    if (mcd->d_ptr->compression && (_value.size() > mcd->d_ptr->compressionThreshold)) {
        flags |= MemcachedPrivate::Compressed;
        _value = qCompress(value, mcd->d_ptr->compressionLevel);
    }

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
                             MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    MemcachedPrivate::Flags flags;
    QByteArray _value = value;

    if (mcd->d_ptr->compression && (_value.size() > mcd->d_ptr->compressionThreshold)) {
        flags |= MemcachedPrivate::Compressed;
        _value = qCompress(value, mcd->d_ptr->compressionLevel);
    }

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
        qCWarning(C_MEMCACHED) << "Failed to replace key " << key
                               << " on group " << groupKey << ": "
                               << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

QByteArray Memcached::get(QByteArrayView key,
                          uint64_t *cas,
                          Cutelyst::Memcached::MemcachedReturnType *returnType)
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
    bool ok = false;

    std::vector<const char *> keys;
    std::vector<size_t> sizes;
    keys.push_back(key.constData());
    sizes.push_back(key.size());
    rt = memcached_mget(mcd->d_ptr->memc, &keys[0], &sizes[0], keys.size());

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
        qCWarning(C_MEMCACHED).nospace() << "Failed to get data for key " << _key
                                         << ": " << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return retData;
}

QByteArray Memcached::getByKey(QByteArrayView groupKey,
                               QByteArrayView key,
                               uint64_t *cas,
                               MemcachedReturnType *returnType)
{
    QByteArray retData;

    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
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
        memcached_result_st *result = memcached_fetch_result(mcd->d_ptr->memc, NULL, &rt);
        if (result) {
            retData = QByteArray(memcached_result_value(result), static_cast<QByteArray::size_type>(memcached_result_length(result)));
            if (cas) {
                *cas = memcached_result_cas(result);
            }
            const MemcachedPrivate::Flags flags{memcached_result_flags(result)};
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
        qCWarning(C_MEMCACHED).nospace() << "Failed to get data for key " << _key
                                         << " on group " << _groupKey << ": "
                                         << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return retData;
}

bool Memcached::remove(QByteArrayView key, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const memcached_return_t rt =
        memcached_delete(mcd->d_ptr->memc, key.constData(), key.size(), 0);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED).nospace() << "Failed to remove data for key " << key
                                         << ": " << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::removeByKey(QByteArrayView groupKey,
                            QByteArrayView key,
                            MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const memcached_return_t rt = memcached_delete_by_key(
        mcd->d_ptr->memc, groupKey.constData(), groupKey.size(), key.constData(), key.size(), 0);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED).nospace() << "Failed to remove data for key " << key
                                         << " on group " << groupKey << ": "
                                         << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::exist(QByteArrayView key, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const memcached_return_t rt = memcached_exist(mcd->d_ptr->memc, key.constData(), key.size());

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED).nospace() << "Failed to check existence of key " << key
                                         << ": " << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::existByKey(QByteArrayView groupKey,
                           QByteArrayView key,
                           MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const memcached_return_t rt = memcached_exist_by_key(
        mcd->d_ptr->memc, groupKey.constData(), groupKey.size(), key.constData(), key.size());

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED).nospace() << "Failed to check existence of key " << key
                                         << " in group " << groupKey << ": "
                                         << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::increment(QByteArrayView key,
                          uint32_t offset,
                          uint64_t *value,
                          MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const memcached_return_t rt =
        memcached_increment(mcd->d_ptr->memc, key.constData(), key.size(), offset, value);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED).nospace() << "Failed to increment key " << key
                                         << " by " << offset << ": "
                                         << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::incrementByKey(QByteArrayView groupKey,
                               QByteArrayView key,
                               uint64_t offset,
                               uint64_t *value,
                               MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
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
        qCWarning(C_MEMCACHED).nospace() << "Failed to increment key " << key
                                         << " in group " << groupKey << " by " << offset
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
                                     MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const memcached_return_t rt = memcached_increment_with_initial(
        mcd->d_ptr->memc, key.constData(), key.size(), offset, initial, expiration, value);

    const bool ok = memcached_success(rt);

    if (!ok) {
        qCWarning(C_MEMCACHED).nospace() << "Failed to increment or initialize key " << key
                                         << " by offset " << offset << " or initial " << initial
                                         << ": " << memcached_strerror(mcd->d_ptr->memc, rt);
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
                                          MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
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
        qCWarning(C_MEMCACHED).nospace() << "Failed to increment or initializes key " << key
                                         << " in group " << groupKey << " by offset " << offset
                                         << " or initial " << initial << ": "
                                         << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::decrement(QByteArrayView key,
                          uint32_t offset,
                          uint64_t *value,
                          MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const memcached_return_t rt =
        memcached_decrement(mcd->d_ptr->memc, key.constData(), key.size(), offset, value);

    const bool ok = memcached_success(rt);

    if (!ok && (rt != MEMCACHED_NOTFOUND)) {
        qCWarning(C_MEMCACHED).nospace() << "Failed to decrement key " << key
                                         << " by " << offset << ": "
                                         << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::decrementByKey(QByteArrayView groupKey,
                               QByteArrayView key,
                               uint64_t offset,
                               uint64_t *value,
                               MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
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
        qCWarning(C_MEMCACHED).nospace() << "Failed to decrement key " << key
                                         << " in group " << groupKey << " by " << offset
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
                                     MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const memcached_return_t rt = memcached_decrement_with_initial(
        mcd->d_ptr->memc, key.constData(), key.size(), offset, initial, expiration, value);

    const bool ok = memcached_success(rt);

    if (!ok) {
        qCWarning(C_MEMCACHED).nospace() << "Failed to decrement of initialize key " << key
                                         << " by offset " << offset << " or initialize " << initial
                                         << ": " << memcached_strerror(mcd->d_ptr->memc, rt);
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
                                          MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
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
        qCWarning(C_MEMCACHED).nospace() << "Failed to decrement or initialize key " << key
                                         << " in group " << groupKey << " by offset " << offset
                                         << " or initial " << initial << ": "
                                         << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::cas(QByteArrayView key,
                    const QByteArray &value,
                    time_t expiration,
                    uint64_t cas,
                    MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    MemcachedPrivate::Flags flags;
    QByteArray _value = value;

    if (mcd->d_ptr->compression && (_value.size() > mcd->d_ptr->compressionThreshold)) {
        flags |= MemcachedPrivate::Compressed;
        _value = qCompress(value, mcd->d_ptr->compressionLevel);
    }

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
        qCWarning(C_MEMCACHED).nospace() << "Failed to compare and set (cas) key " << key
                                         << ": " << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::casByKey(QByteArrayView groupKey,
                         QByteArrayView key,
                         const QByteArray &value,
                         time_t expiration,
                         uint64_t cas,
                         MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    MemcachedPrivate::Flags flags;
    QByteArray _value = value;

    if (mcd->d_ptr->compression && (_value.size() > mcd->d_ptr->compressionThreshold)) {
        flags |= MemcachedPrivate::Compressed;
        _value = qCompress(value, mcd->d_ptr->compressionLevel);
    }

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
        qCWarning(C_MEMCACHED).nospace() << "Failed to compare and set (cas) key " << key
                                         << " in group " << groupKey << ": "
                                         << memcached_strerror(mcd->d_ptr->memc, rt);
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
        qCWarning(C_MEMCACHED) << "Failed to flush buffers:"
                               << memcached_strerror(mcd->d_ptr->memc, rt);
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
        qCWarning(C_MEMCACHED) << "Failed to wipe (flush) server content:"
                               << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

QHash<QByteArray, QByteArray> Memcached::mget(const QByteArrayList &keys,
                                              QHash<QByteArray, uint64_t> *casValues,
                                              MemcachedReturnType *returnType)
{
    QHash<QByteArray, QByteArray> ret;

    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return ret;
    }

    if (keys.empty()) {
        qCWarning(C_MEMCACHED) << "Can not get multiple values without a list of keys.";
        if (returnType) {
            *returnType = Memcached::BadKeyProvided;
        }
        return ret;
    }

    std::vector<char *> _keys;
    _keys.reserve(keys.size());
    std::vector<size_t> _keysSizes;
    _keysSizes.reserve(keys.size());

    for (const auto &key : keys) {
        char *data = new char[key.size() + 1];
        qstrcpy(data, key.data());
        _keys.push_back(data);
        _keysSizes.push_back(key.size());
    }

    memcached_return_t rt;
    bool ok = false;

    rt = memcached_mget(mcd->d_ptr->memc, &_keys[0], &_keysSizes[0], _keys.size());

    if (memcached_success(rt)) {
        ok = true;
        ret.reserve(keys.size());
        while ((rt != MEMCACHED_END) && (rt != MEMCACHED_NOTFOUND)) {
            memcached_result_st *result = memcached_fetch_result(mcd->d_ptr->memc, NULL, &rt);
            if (result) {
                const QByteArray rk = QByteArray(memcached_result_key_value(result),
                                                 memcached_result_key_length(result));
                QByteArray rd(memcached_result_value(result), memcached_result_length(result));
                if (casValues) {
                    casValues->insert(rk, memcached_result_cas(result));
                }
                MemcachedPrivate::Flags flags =
                    MemcachedPrivate::Flags(memcached_result_flags(result));
                if (flags.testFlag(MemcachedPrivate::Compressed)) {
                    rd = qUncompress(rd);
                }
                ret.insert(rk, rd);
            }
            memcached_result_free(result);
        }
    }

    for (char *c : _keys) {
        delete[] c;
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
                                                   MemcachedReturnType *returnType)
{
    QHash<QByteArray, QByteArray> ret;

    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return ret;
    }

    if (groupKey.isEmpty()) {
        qCWarning(C_MEMCACHED) << "Can not get multiple values from specific server when groupKey is empty.";
        if (returnType) {
            *returnType = Memcached::BadKeyProvided;
        }
        return ret;
    }

    if (keys.empty()) {
        qCWarning(C_MEMCACHED) << "Can not get multiple values without a list of keys.";
        if (returnType) {
            *returnType = Memcached::BadKeyProvided;
        }
        return ret;
    }

    std::vector<char *> _keys;
    _keys.reserve(keys.size());
    std::vector<size_t> _keysSizes;
    _keysSizes.reserve(keys.size());

    for (const auto &key : keys) {
        char *data = new char[key.size() + 1];
        strcpy(data, key.data());
        _keys.push_back(data);
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
            memcached_result_st *result = memcached_fetch_result(mcd->d_ptr->memc, NULL, &rt);
            if (result) {
                const QByteArray rk = QByteArray(memcached_result_key_value(result),
                                                 memcached_result_key_length(result));
                QByteArray rd(memcached_result_value(result), memcached_result_length(result));
                if (casValues) {
                    casValues->insert(rk, memcached_result_cas(result));
                }
                const MemcachedPrivate::Flags flags{memcached_result_flags(result)};
                if (flags.testFlag(MemcachedPrivate::Compressed)) {
                    rd = qUncompress(rd);
                }
                ret.insert(rk, rd);
            }
            memcached_result_free(result);
        }
    }

    for (char *c : _keys) {
        delete[] c;
    }

    if (!ok) {
        qCWarning(C_MEMCACHED).nospace() << "Failed to get values for multiple keys in group " << groupKey
                                         << ": " << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ret;
}

bool Memcached::touch(QByteArrayView key, time_t expiration, MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
        return false;
    }

    const memcached_return_t rt =
        memcached_touch(mcd->d_ptr->memc, key.constData(), key.size(), expiration);

    const bool ok = memcached_success(rt);

    if (!ok) {
        qCWarning(C_MEMCACHED).nospace() << "Failed to touch key " << key
                                         << " with new expiration time " << expiration
                                         << ": " << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

bool Memcached::touchByKey(QByteArrayView groupKey,
                           QByteArrayView key,
                           time_t expiration,
                           MemcachedReturnType *returnType)
{
    if (!mcd) {
        qCCritical(C_MEMCACHED) << "Memcached plugin not registered";
        if (returnType) {
            *returnType = Memcached::PluginNotRegisterd;
        }
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
        qCWarning(C_MEMCACHED).nospace() << "Failed to touch key " << key
                                         << " in group " << groupKey
                                         << " with new expiration time " << expiration
                                         << ": " << memcached_strerror(mcd->d_ptr->memc, rt);
    }

    MemcachedPrivate::setReturnType(returnType, rt);

    return ok;
}

QString Memcached::errorString(Context *c, MemcachedReturnType rt)
{
    switch (rt) {
    case Memcached::Success:
        return c->translate("Cutelyst::Memcached", "The request was successfully executed.");
    case Memcached::Failure:
        return c->translate("Cutelyst::Memcached",
                            "An unknown failure has occurred in the Memcached server.");
    case Memcached::HostLookupFailure:
        return c->translate("Cutelyst::Memcached", "A DNS failure has occurred.");
    case Memcached::ConnectionFailure:
        return c->translate(
            "Cutelyst::Memcached",
            "An unknown error has occurred while trying to connect to a Memcached server.");
    case Memcached::WriteFailure:
        return c->translate("Cutelyst::Memcached",
                            "An error has occurred while trying to write to a Memcached server.");
    case Memcached::ReadFailure:
        return c->translate("Cutelyst::Memcached",
                            "An error has occurred while trying to read from a Memcached server.");
    case Memcached::UnknownReadFailure:
        return c->translate(
            "Cutelyst::Memcached",
            "An unknown error has occurred while trying to read from a Memcached server. This only "
            "occures when either there is a bug in the server, or in rare cases where an ethernet "
            "NIC is reporting dubious information.");
    case Memcached::ProtocolError:
        return c->translate("Cutelyst::Memcached",
                            "An unknown error has occurred in the Memcached protocol.");
    case Memcached::ClientError:
        return c->translate("Cutelyst::Memcached",
                            "An unknown Memcached client error has occurred internally.");
    case Memcached::ServerError:
        return c->translate("Cutelyst::Memcached",
                            "An unknown error has occurred in the Memcached server.");
    case Memcached::Error:
        return c->translate("Cutelyst::Memcached", "A general error occurred.");
    case Memcached::DataExists:
        return c->translate("Cutelyst::Memcached", "The data for the given key alrey exists.");
    case Memcached::DataDoesNotExist:
        return c->translate("Cutelyst::Memcached",
                            "The data requested with the key given was not found.");
    case Memcached::NotStored:
        return c->translate("Cutelyst::Memcached", "The request to store an object failed.");
    case Memcached::Stored:
        return c->translate("Cutelyst::Memcached",
                            "The requested object has been successfully stored on the server.");
    case Memcached::NotFound:
        return c->translate("Cutelyst::Memcached", "The object requested was not found.");
    case Memcached::MemoryAllocationFailure:
        return c->translate("Cutelyst::Memcached",
                            "An error has occurred while trying to allocate memory.");
    case Memcached::PartialRead:
        return c->translate("Cutelyst::Memcached",
                            "The read operation was only partcially successful.");
    case Memcached::SomeErrors:
        return c->translate("Cutelyst::Memcached",
                            "A multi request has been made, and some underterminate number of "
                            "errors have occurred.");
    case Memcached::NoServers:
        return c->translate("Cutelyst::Memcached",
                            "No servers have been added to the Memcached plugin.");
    case Memcached::End:
        return c->translate(
            "Cutelyst::Memcached",
            "The Memcached server has completed returning all of the objects requested.");
    case Memcached::Deleted:
        return c->translate("Cutelyst::Memcached",
                            "The object requested by the key has been deleted.");
    case Memcached::Stat:
        return c->translate("Cutelyst::Memcached",
                            "A “stat” command has been returned in the protocol.");
    case Memcached::Errno:
        return c->translate("Cutelyst::Memcached",
                            "An error has occurred in the driver which has set errno.");
    case Memcached::NotSupported:
        return c->translate("Cutelyst::Memcached",
                            "The given method is not supported in the Memcached server.");
    case Memcached::FetchNotFinished:
        return c->translate("Cutelyst::Memcached",
                            "A request has been made, but the Memcached server has not finished "
                            "the fetch of the last request.");
    case Memcached::Timeout:
        return c->translate("Cutelyst::Memcached", "The operation has timed out.");
    case Memcached::Buffered:
        return c->translate("Cutelyst::Memcached", "The request has been buffered.");
    case Memcached::BadKeyProvided:
        return c->translate("Cutelyst::Memcached", "The key provided is not a valid key.");
    case Memcached::InvalidHostProtocol:
        return c->translate(
            "Cutelyst::Memcached",
            "The Memcached server you are connecting to has an invalid protocol. Most likely you "
            "are connecting to an older server that does not speak the binary protocol.");
    case Memcached::ServerMarkedDead:
        return c->translate("Cutelyst::Memcached",
                            "The requested Memcached server has been marked dead.");
    case Memcached::UnknownStatKey:
        return c->translate("Cutelyst::Memcached",
                            "The Memcached server you are communicating with has a stat key which "
                            "has not be defined in the protocol.");
    case Memcached::E2Big:
        return c->translate("Cutelyst::Memcached",
                            "Item is too large for the Memcached server to store.");
    case Memcached::InvalidArguments:
        return c->translate("Cutelyst::Memcached",
                            "The arguments supplied to the given function were not valid.");
    case Memcached::KeyTooBig:
        return c->translate(
            "Cutelyst::Memcached",
            "The key that has been provided is too large for the given Memcached server.");
    case Memcached::AuthProblem:
        return c->translate("Cutelyst::Memcached",
                            "An unknown issue has occurred during SASL authentication.");
    case Memcached::AuthFailure:
        return c->translate("Cutelyst::Memcached",
                            "The credentials provided are not valid for this Memcached server.");
    case Memcached::AuthContinue:
        return c->translate("Cutelyst::Memcached", "Authentication has been paused.");
    case Memcached::ParseError:
        return c->translate(
            "Cutelyst::Memcached",
            "An error has occurred while trying to parse the configuration string.");
    case Memcached::ParseUserError:
        return c->translate("Cutelyst::Memcached",
                            "An error has occurred in parsing the configuration string.");
    case Memcached::Deprecated:
        return c->translate("Cutelyst::Memcached",
                            "The method that was requested has been deprecated.");
    case Memcached::PluginNotRegisterd:
        return c->translate(
            "Cutelyst::Memcached",
            "The Cutelyst Memcached plugin has not been registered to the application.");
    default:
        return c->translate("Cutelyst::Memcached", "An unknown error has occurred.");
    }
}

QVersionNumber Memcached::libMemcachedVersion()
{
    return QVersionNumber::fromString(QLatin1String(memcached_lib_version()));
}

// clang-format off
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
// clang-format on

void MemcachedPrivate::setReturnType(Memcached::MemcachedReturnType *rt1, memcached_return_t rt2)
{
    if (rt1) {
        *rt1 = MemcachedPrivate::returnTypeConvert(rt2);
    }
}

#include "moc_memcached.cpp"
