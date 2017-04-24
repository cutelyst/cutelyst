/*
 * Copyright (C) 2015-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef SESSIONSTOREFILE_H
#define SESSIONSTOREFILE_H

#include <Cutelyst/Plugins/Session/session.h>
#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class SessionStoreFilePrivate;
class CUTELYST_PLUGIN_SESSION_EXPORT SessionStoreFile : public SessionStore
{
    Q_OBJECT
public:
    /**
     * Constructs a new session store file object with the given parent.
     */
    explicit SessionStoreFile(QObject *parent = nullptr);
    ~SessionStoreFile();

    /**
     * Reimplemented from SessionStore::getSessionData().
     */
    virtual QVariant getSessionData(Context *c, const QString &sid, const QString &key, const QVariant &defaultValue) final;

    /**
     * Reimplemented from SessionStore::storeSessionData().
     */
    virtual bool storeSessionData(Context *c, const QString &sid, const QString &key, const QVariant &value) final;

    /**
     * Reimplemented from SessionStore::deleteSessionData().
     */
    virtual bool deleteSessionData(Context *c, const QString &sid, const QString &key) final;

    /**
     * Reimplemented from SessionStore::deleteExpiredSessions().
     */
    virtual bool deleteExpiredSessions(Context *c, quint64 expires) final;
};

}

#endif // SESSIONSTOREFILE_H
