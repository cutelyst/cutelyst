/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
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
    inline SessionPrivate(Session *q)
        : q_ptr(q)
    {
    }

    static inline QByteArray generateSessionId();
    static QByteArray loadSessionId(Context *c, const QByteArray &sessionName);
    static QByteArray getSessionId(Context *c, const QByteArray &sessionName);
    static QByteArray createSessionIdIfNeeded(Session *session, Context *c, qint64 expires);
    static inline QByteArray createSessionId(Session *session, Context *c, qint64 expires);
    static void _q_saveSession(Context *c);
    static void deleteSession(Session *session, Context *c, const QString &reason);
    static inline void deleteSessionId(Session *session, Context *c, const QByteArray &sid);
    static QVariant loadSession(Context *c);
    static bool validateSessionId(QByteArrayView id);
    static qint64 extendSessionExpires(Session *session, Context *c, qint64 expires);
    static qint64
        getStoredSessionExpires(Session *session, Context *c, const QByteArray &sessionid);

    static inline QVariant initializeSessionData(Session *session, Context *c);
    static void saveSessionExpires(Context *c);
    static QVariant loadSessionExpires(Session *session, Context *c, const QByteArray &sessionId);
    static inline qint64 initialSessionExpires(Session *session, Context *c);
    static inline qint64
        calculateInitialSessionExpires(Session *session, Context *c, const QByteArray &sessionId);
    static inline qint64
        resetSessionExpires(Session *session, Context *c, const QByteArray &sessionId);

    static inline void updateSessionCookie(Context *c, const QNetworkCookie &updated);
    static inline QNetworkCookie makeSessionCookie(Session *session,
                                                   Context *c,
                                                   const QByteArray &sid,
                                                   const QDateTime &expires);
    static inline void
        extendSessionId(Session *session, Context *c, const QByteArray &sid, qint64 expires);
    static inline void setSessionId(Session *session, Context *c, const QByteArray &sid);
    QVariant config(const QString &key, const QVariant &defaultValue = {}) const;

    Session *q_ptr;

    qint64 sessionExpires  = 7200;
    qint64 expiryThreshold = 0;
    std::unique_ptr<SessionStore> store;
    QVariantMap loadedConfig;
    QVariantMap defaultConfig;
    QByteArray sessionName;
    QNetworkCookie::SameSite cookieSameSite = QNetworkCookie::SameSite::Strict;
    bool cookieHttpOnly                     = true;
    bool cookieSecure                       = false;
    bool verifyAddress                      = false;
    bool verifyUserAgent                    = false;
};

} // namespace Cutelyst

#endif // SESSION_P_H
