/*
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
 * This session store saves session data to a <a href="http://memcached.org">Memcached</a> server using <a href="http://libmemcached.org/libMemcached.html">libMemcached</a>.
 *
 * This is completely optional. To build it, you need <a href="https://www.freedesktop.org/wiki/Software/pkg-config">pkg-config</a> and <a href="http://libmemcached.org/libMemcached.html">libMemcached</a> in your build environment. If pkg-config and libMemcached can not be found, this session store will be silently disabled and not be build.
 *
 * @deprecated This class is deprecated. Use the new MemcachedSessionStore plugin instead.
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
    Q_DECL_DEPRECATED_X("Use MemcachedSessionStore plugin instead.") explicit SessionStoreMemcached(QObject *parent = nullptr);

    /*!
     * \brief Constructs a SessionStoreMemcached object with given \a config and \a parent.
     *
     * Use the \a config string as described by the <a href="http://docs.libmemcached.org/libmemcached_configuration.html">libmemcached documentation</a> to
     * construct a SessionStoreMemcached object with more specific configuration.
     */
    Q_DECL_DEPRECATED_X("Use MemcachedSessionStore plugin instead.") SessionStoreMemcached(const QString &config, QObject *parent = nullptr);

    /*!
     * \brief Deconstructs the SessionStoreMemcached object.
     */
    ~SessionStoreMemcached();

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

protected:
    SessionStoreMemcachedPrivate *d_ptr;
};

}

#endif // SESSIONSTOREMEMCACHED_H
