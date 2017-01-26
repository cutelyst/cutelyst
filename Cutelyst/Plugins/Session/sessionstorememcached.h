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

/*!
 * \brief Memcached session store.
 *
 * This session store saves session data to a \link http://memcached.org/ Memcached \endlink server using \link http://libmemcached.org/libMemcached.html libmemcached \endlink.
 *
 * \par Usage example with custom configuration
 *
 * \code{.cpp}
 * #include <Cutelyst/Plugins/Session/Session>
 * #include <Cutelyst/Plugins/Session/sessionstorememcached.h>
 *
 * bool MyCutelystApp::init()
 * {
 *      // other initialization stuff
 *      // ...
 *
 *      auto session = new Session(this);
 *      sess->setStorage(new SessionStoreMemcached(QStringLiteral("--SOCKET=\"/var/run/memcached/memcached.sock\"")));
 *
 *      // maybe more initialization stuff
 *      // ...
 * }
 * \endcode
 */
class CUTELYST_PLUGIN_SESSION_EXPORT SessionStoreMemcached : public SessionStore
{
    Q_OBJECT
public:
    /*!
     * \brief Constructs a default SessionStoreMemcached object with given \a parent.
     *
     * This default created session store will connect to the memcached server running
     * on localhost port 11211.
     */
    explicit SessionStoreMemcached(QObject *parent = nullptr);

    /*!
     * \brief Constructs a SessionStoreMemcached object with given \a config and \a parent.
     *
     * Use the \a config string as described by the \link http://docs.libmemcached.org/libmemcached_configuration.html libmemcached documentation \endlink to
     * construct a SessionStoreMemcached object with more specific configuration.
     */
    SessionStoreMemcached(const QString &config, QObject *parent = nullptr);

    /*!
     * \brief Deconstructs the SessionStoreMemcached object.
     */
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
