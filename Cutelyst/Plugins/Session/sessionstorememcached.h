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
#ifndef SESSIONSTOREMEMCACHED_H
#define SESSIONSTOREMEMCACHED_H

#include <Cutelyst/Plugins/Session/session.h>
#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class SessionStoreMemcachedPrivate;

class CUTELYST_PLUGIN_SESSION_EXPORT SessionStoreMemcached : public SessionStore
{
    Q_OBJECT
public:
    explicit SessionStoreMemcached(QObject *parent = nullptr);
    SessionStoreMemcached(const QString &config, QObject *parent = nullptr);
    ~SessionStoreMemcached();

    QVariant getSessionData(Context *c, const QString &sid, const QString &key, const QVariant &defaultValue) final;
    bool storeSessionData(Context *c, const QString &sid, const QString &key, const QVariant &value) final;
    bool deleteSessionData(Context *c, const QString &sid, const QString &key) final;
    bool deleteExpiredSessions(Context *c, quint64 expires) final;

protected:
    SessionStoreMemcachedPrivate *d_ptr;
};

}

#endif // SESSIONSTOREMEMCACHED_H
