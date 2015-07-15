/*
 * Copyright (C) 2013 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef CSESSION_H
#define CSESSION_H

#include <QVariant>

#include <Cutelyst/plugin.h>

namespace Cutelyst {

class Context;
class SessionStore : public QObject {
    Q_OBJECT
public:
    explicit SessionStore(QObject *parent = 0);

    virtual QVariant getSessionData(Context *c, const QString &sid, const QString &key, const QVariant &defaultValue = QVariant()) = 0;
    virtual bool storeSessionData(Context *c, const QString &sid, const QString &key, const QVariant &value) = 0;
    virtual bool deleteSessionData(Context *c, const QString &sid, const QString &key) = 0;
};

class SessionPrivate;
class Session : public Plugin
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Session)
public:
    Session(Application *parent);
    virtual ~Session();

    /**
     * If config has
     * [Plugin_Session]
     * expires = 1234
     * it will change de default expires which is 7200 (two hours)
     */
    virtual bool setup(Application *app) Q_DECL_FINAL;

    void setStorage(SessionStore *store);

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

    static QVariant value(Context *c, const QString &key, const QVariant &defaultValue = QVariant());
    static void setValue(Context *c, const QString &key, const QVariant &value);
    static void deleteValue(Context *c, const QString &key);

    static bool isValid(Context *c);

protected:
    SessionPrivate *d_ptr;
};

}

#endif // CSESSION_H
