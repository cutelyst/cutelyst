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
    virtual bool setup(Application *app);

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
    static void setExpires(Context *c, quint64 expires);

    static QVariant value(Context *c, const QString &key, const QVariant &defaultValue = QVariant());
    static void setValue(Context *c, const QString &key, const QVariant &value);
    static void deleteValue(Context *c, const QString &key);

    static bool isValid(Context *c);

protected:
    /**
     * This method is used to call the storage class and retrieve
     * the session, the default implementation does that by using a file
     */
    virtual QVariantHash retrieveSession(const QString &sessionId, quint64 &expires) const;

    /**
     * This methos is used to call the storage class and persist
     * the session data, the default implementation does that by using a file
     */
    virtual void persistSession(const QString &sessionId, const QVariant &data, quint64 expires) const;

protected:
    SessionPrivate *d_ptr;
};

}

#endif // CSESSION_H
