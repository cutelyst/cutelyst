/*
 * Copyright (C) 2014-2016 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef SESSION_P_H
#define SESSION_P_H

#include "session.h"

#include <QtNetwork/QNetworkCookie>

namespace Cutelyst {

class SessionPrivate
{
    Q_DECLARE_PUBLIC(Session)
public:
    inline SessionPrivate(Session *q) : q_ptr(q) { }

    static inline QString generateSessionId();
    static QString loadSessionId(Context *c, const QString &sessionName);
    static QString getSessionId(Context *c, const QString &sessionName);
    static QString createSessionIdIfNeeded(Session *session, Context *c, qint64 expires);
    static inline QString createSessionId(Session *session, Context *c, qint64 expires);
    static void _q_saveSession(Context *c);
    static void deleteSession(Session *session, Context *c, const QString &reason);
    static inline void deleteSessionId(Session *session, Context *c, const QString &sid);
    static QVariant loadSession(Context *c);
    static bool validateSessionId(const QString &id);
    static qint64 extendSessionExpires(Session *session, Context *c, qint64 expires);
    static qint64 getStoredSessionExpires(Session *session, Context *c, const QString &sessionid);

    static inline QVariant initializeSessionData(Session *session, Context *c);
    static void saveSessionExpires(Context *c);
    static QVariant loadSessionExpires(Session *session, Context *c, const QString &sessionId);
    static inline qint64 initialSessionExpires(Session *session, Context *c);
    static inline qint64 calculateInitialSessionExpires(Session *session, Context *c, const QString &sessionId);
    static inline qint64 resetSessionExpires(Session *session, Context *c, const QString &sessionId);

    static inline void updateSessionCookie(Context *c, const QNetworkCookie &updated);
    static inline QNetworkCookie makeSessionCookie(Session *session, Context *c, const QString &sid, const QDateTime &expires);
    static inline void extendSessionId(Session *session, Context *c, const QString &sid, qint64 expires);
    static inline void setSessionId(Session *session, Context *c, const QString &sid);

    Session *q_ptr;

    qint64 sessionExpires = 7200;
    qint64 expiryThreshold = 0;
    SessionStore *store = nullptr;
    QString sessionName;
    bool cookieHttpOnly = true;
    bool cookieSecure = false;
    bool verifyAddress = false;
    bool verifyUserAgent = false;
};

}

#endif // SESSION_P_H
