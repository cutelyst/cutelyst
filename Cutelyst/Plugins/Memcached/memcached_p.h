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

#ifndef CUTELYSTMEMCACHED_P_H
#define CUTELYSTMEMCACHED_P_H

#include "memcached.h"

#include <libmemcached/memcached.h>
#include <QString>
#include <QMap>
#include <QFlags>

namespace Cutelyst {

class MemcachedPrivate
{
public:
    MemcachedPrivate() {}

    ~MemcachedPrivate()
    {
        if (memc) {
            if (saslEnabled) {
                memcached_destroy_sasl_auth_data(memc);
            }
            memcached_free(memc);
        }
    }

    enum Flag : quint32 {
        NoFlags = 0x0,
        Compressed = 0x1
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    static void _q_postFork(Application *app);

    static Memcached::MemcachedReturnType returnTypeConvert(memcached_return_t rt);
    static bool checkInput(Memcached *ptr, const QString &key, const QString &action, Memcached::MemcachedReturnType *rt);
    static bool checkInputByKey(Memcached *ptr, const QString &groupKey, const QString &key, const QString &action, Memcached::MemcachedReturnType *rt);
    static void setReturnType(Memcached::MemcachedReturnType *rt1, memcached_return_t rt2);

    QMap<int,std::pair<QString,quint16>> servers;
    memcached_st *memc = nullptr;

    bool compression = false;
    int compressionThreshold = 100;
    int compressionLevel = -1;
    bool saslEnabled = false;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Cutelyst::MemcachedPrivate::Flags)

#endif // CUTELYSTMEMCACHED_P_H
