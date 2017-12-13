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
#ifndef CUTELYSTMEMCACHEDSESSIONSTORE_H
#define CUTELYSTMEMCACHEDSESSIONSTORE_H

#include <Cutelyst/Plugins/Session/session.h>
#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class Application;
class MemcachedSessionStorePrivate;

/**
 * @brief Memcached based session store.
 *
 * This session store saves session data to <A HREF="http://memcached.org">Memcached</A> servers using the Memcached plugin.
 * It obsoletes the SessionStoreMemcached class.
 *
 * @note The memcached server does not guarantee the existence of the session data. It might for example delete the data because
 * it runs out of memory and deletes session data. So be careful when using this plugin to store sessions.
 *
 * <H3>Configuration</h3>
 *
 * The %MemcachedSessionStore plugin can be configured in the cutelyst configuration file in the @c Cutelyst_MemcachedSessionStore_Plugin section.
 *
 * Currently there are the following configuration options:
 * @li @a group_key - string value, defines a group key to store the data on a specific server (default: empty)
 *
 * <H4>Configuration example</H4>
 *
 * @code{.ini}
 * [Cutelyst_MemcachedSessionStore_Plugin]
 * group_key=sessions
 * @endcode
 *
 * <H3>Usage example</H3>
 *
 * @code{.cpp}
 * #include <Cutelyst/Plugins/Memcached/Memcached>
 * #include <Cutelyst/Plugins/MemcachedSessionStore/MemcachedSessionStore>
 *
 * bool MyCutelystApp::init()
 * {
 *     // other initilization stuff
 *     // ...
 *
 *     new Memcached(this);
 *     auto sess = new Session(this);
 *     sess->setStorage(new MemcachedSessionStore(this));
 *
 *     // maybe more initilization stuff
 *     // ...
 * }
 * @endcode
 *
 * <H3>Build requirements</H3>
 *
 * To build this plugin you need the development and header files for <A HREF="http://libmemcached.org">libmemcached</A>
 * and have to enable the Memcached plugin. If the Memcached plugin is enabled, the %MemcachedSessionStore plugin will be
 * enabled automatically, too. To disable the build of the %MemcachedSessionStore plugin, run cmake with
 * <CODE>-DPLUGIN_MEMCACHEDSESSIONSTORE:BOOL=OFF</CODE>.
 *
 * @since Cutelyst 1.11.0
 */
class CUTELYST_PLUGIN_MEMCACHEDSESSIONSTORE_EXPORT MemcachedSessionStore : public SessionStore
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(MemcachedSessionStore)
public:
    /**
     * Constructs a new MemcachedSessionStore object with the given @a parent and @a app.
     * The pointer to the Application object is used to load the plugin configuration from
     * the configuration file section @c Cutelyst_MemcachedSessionStore_Plugin.
     */
    MemcachedSessionStore(Application *app, QObject *parent = nullptr);

    /**
     * Deconstructs the MemcachedSessionStore object
     */
    ~MemcachedSessionStore();

    virtual QVariant getSessionData(Context *c, const QString &sid, const QString &key, const QVariant &defaultValue) final;

    virtual bool storeSessionData(Context *c, const QString &sid, const QString &key, const QVariant &value) final;

    virtual bool deleteSessionData(Context *c, const QString &sid, const QString &key) final;

    virtual bool deleteExpiredSessions(Context *c, quint64 expires) final;

    /**
     * Sets the @a groupKey to define the servers to store the sessions on.
     * This can also be set in the configuration file.
     */
    void setGroupKey(const QString &groupKey);

protected:
    QScopedPointer<MemcachedSessionStorePrivate> d_ptr;
};

}

#endif // CUTELYSTMEMCACHEDSESSIONSTORE_H
