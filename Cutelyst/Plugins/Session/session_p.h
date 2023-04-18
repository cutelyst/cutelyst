/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef SESSION_P_H
#define SESSION_P_H

#include "session.h"
#if (QT_VERSION < QT_VERSION_CHECK(6, 1, 0))
#    include "cookie.h"
#endif

#include <QtNetwork/QNetworkCookie>

namespace Cutelyst {

class SessionPrivate
{
    Q_DECLARE_PUBLIC(Session)
public:
    inline SessionPrivate(Session *q)
        : q_ptr(q)
    {
    }

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
#if (QT_VERSION < QT_VERSION_CHECK(6, 1, 0))
    static inline void updateSessionCuteCookie(Context *c, const Cookie &updated);
    static inline Cookie makeSessionCuteCookie(Session *session, Context *c, const QString &sid, const QDateTime &expires);
#endif
    static inline void extendSessionId(Session *session, Context *c, const QString &sid, qint64 expires);
    static inline void setSessionId(Session *session, Context *c, const QString &sid);

    Session *q_ptr;

    qint64 sessionExpires  = 7200;
    qint64 expiryThreshold = 0;
    SessionStore *store    = nullptr;
    QString sessionName;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 1, 0))
    QNetworkCookie::SameSite cookieSameSite = QNetworkCookie::SameSite::Strict;
#else
    Cookie::SameSite cookieSameSite = Cookie::SameSite::Strict;
#endif
    bool cookieHttpOnly  = true;
    bool cookieSecure    = false;
    bool verifyAddress   = false;
    bool verifyUserAgent = false;
};

} // namespace Cutelyst

#endif // SESSION_P_H
