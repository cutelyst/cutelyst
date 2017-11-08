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

namespace Cutelyst {

class MemcachedPrivate
{
public:
    MemcachedPrivate() {}

    ~MemcachedPrivate()
    {
        if (memc) {
            memcached_free(memc);
        }
    }

    static void _q_postFork(Application *app);

    static Memcached::MemcachedReturnType returnTypeConvert(memcached_return_t rt);
    inline static bool checkInput(Memcached *ptr, const QString &key, const QString &action, Memcached::MemcachedReturnType *rt);
    inline static bool checkInputByKey(Memcached *ptr, const QString &groupKey, const QString &key, const QString &action, Memcached::MemcachedReturnType *rt);
    inline static void setReturnType(Memcached::MemcachedReturnType *rt1, memcached_return_t rt2);

    QMap<int,std::pair<QString,quint16>> servers;
    memcached_st *memc = nullptr;
};

inline bool MemcachedPrivate::checkInput(Memcached *ptr, const QString &key, const QString &action, Memcached::MemcachedReturnType *rt)
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

inline bool MemcachedPrivate::checkInputByKey(Memcached *ptr, const QString &groupKey, const QString &key, const QString &action, Memcached::MemcachedReturnType *rt)
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

inline void MemcachedPrivate::setReturnType(Memcached::MemcachedReturnType *rt1, memcached_return_t rt2)
{
    if (rt1) {
        *rt1 = MemcachedPrivate::returnTypeConvert(rt2);
    }
}

}

#endif // CUTELYSTMEMCACHED_P_H
