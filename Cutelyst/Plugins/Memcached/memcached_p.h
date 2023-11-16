/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CUTELYSTMEMCACHED_P_H
#define CUTELYSTMEMCACHED_P_H

#include "memcached.h"

#include <libmemcached/memcached.h>

#include <QFlags>
#include <QMap>
#include <QString>

namespace Cutelyst {

class MemcachedPrivate
{
public:
    MemcachedPrivate() {}

    ~MemcachedPrivate()
    {
        if (memc) {
#ifdef LIBMEMCACHED_WITH_SASL_SUPPORT
#    if LIBMEMCACHED_WITH_SASL_SUPPORT == 1
            if (saslEnabled) {
                memcached_destroy_sasl_auth_data(memc);
            }
#    endif
#endif
            memcached_free(memc);
        }
    }

    enum Flag : quint32 { NoFlags = 0x0, Compressed = 0x1 };
    Q_DECLARE_FLAGS(Flags, Flag)

    static Memcached::MemcachedReturnType returnTypeConvert(memcached_return_t rt);
    static void setReturnType(Memcached::MemcachedReturnType *rt1, memcached_return_t rt2);
    static bool isRegistered(Memcached *ptr, Memcached::MemcachedReturnType *rt);

    static constexpr uint16_t defaultPort{11211};
    static constexpr int defaultCompressionThreshold{100};

    QMap<int, std::pair<QString, quint16>> servers;
    memcached_st *memc = nullptr;

    bool compression         = false;
    int compressionThreshold = defaultCompressionThreshold;
    int compressionLevel     = -1;
    bool saslEnabled         = false;

    QVariantMap defaultConfig;
};

} // namespace Cutelyst

Q_DECLARE_OPERATORS_FOR_FLAGS(Cutelyst::MemcachedPrivate::Flags)

#endif // CUTELYSTMEMCACHED_P_H
