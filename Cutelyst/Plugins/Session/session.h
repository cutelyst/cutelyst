/*
 * Copyright (C) 2013-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef CSESSION_H
#define CSESSION_H

#include <QVariant>

#include <Cutelyst/plugin.h>
#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class Context;
class CUTELYST_PLUGIN_SESSION_EXPORT SessionStore : public QObject {
    Q_OBJECT
public:
    /**
     * Constructs a new session store object with the given parent.
     */
    explicit SessionStore(QObject *parent = nullptr);

    /**
     * Returns the session data for the given session id sid and key, if key does not exist returns defaultValue.
     */
    virtual QVariant getSessionData(Context *c, const QString &sid, const QString &key, const QVariant &defaultValue = QVariant()) = 0;

    /**
     * Stores the session data for the given session id sid and key to value.
     */
    virtual bool storeSessionData(Context *c, const QString &sid, const QString &key, const QVariant &value) = 0;

    /**
     * Removes all session data for the given session id sid and key.
     */
    virtual bool deleteSessionData(Context *c, const QString &sid, const QString &key) = 0;

    /**
     * Removes all expired sessions which are above expires.
     */
    virtual bool deleteExpiredSessions(Context *c, quint64 expires) = 0;
};

class SessionPrivate;
class CUTELYST_PLUGIN_SESSION_EXPORT Session : public Plugin
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Session)
public:
    /**
     * Constructs a new session object with the given parent.
     */
    Session(Application *parent);
    virtual ~Session();

    /**
     * If config has
     * [Cutelyst_Session_Plugin]
     * expires = 1234
     * it will change the default expires which is 7200 (two hours)
     */
    virtual bool setup(Application *app) final;

    /**
     * Sets the session storage
     */
    void setStorage(SessionStore *store);

    /**
     * Returns the session storage
     */
    SessionStore *storage() const;

    /**
     * Returns the current session id or null if
     * there is no current session
     */
    static QString id(Context *c);

    /**
     * This method returns the time when the current session will expire, or 0 if there is no current session.
     * If there is a session and it already expired, it will delete the session and return 0 as well.
     */
    static quint64 expires(Context *c);

    /**
     * change the session expiration time for this session
     *
     * Note that this only works to set the session longer than the config setting.
     */
    static void changeExpires(Context *c, quint64 expires);

    /**
     * This method is used to invalidate a session. It takes an optional parameter
     * which will be saved in deleteReason if provided.
     *
     * NOTE: This method will also delete your flash data.
     */
    static void deleteSession(Context *c, const QString &reason = QString());

    /**
     * This method contains a string with the reason a session was deleted. Possible values include:
     * - session expired
     * - address mismatch
     * - user agent mismatch
     */
    static QString deleteReason(Context *c);

    /**
     * Returns the value for session key. If the session key doesn't exist, returns defaultValue.
     */
    static QVariant value(Context *c, const QString &key, const QVariant &defaultValue = QVariant());

    /**
     * Sets the value for session key to value. If the key already exists, the previous value is overwritten.
     */
    static void setValue(Context *c, const QString &key, const QVariant &value);

    /**
     * Removes the session key.
     */
    static void deleteValue(Context *c, const QString &key);

    /**
     * Removes all session keys.
     */
    static void deleteValues(Context *c, const QStringList &keys);

    /**
     * Returns true if the session is valid.
     */
    static bool isValid(Context *c);

protected:
    SessionPrivate *d_ptr;

private:
    Q_PRIVATE_SLOT(d_func(), void _q_saveSession(Context*))
    Q_PRIVATE_SLOT(d_func(), void _q_postFork(Application*))
};

}

#endif // CSESSION_H
