/*
 * Copyright (C) 2014-2015 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef SESSION_P_H
#define SESSION_P_H

#include "session.h"

#include <QRegularExpression>

namespace Cutelyst {

class SessionPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(Session)
public:
    Session *q_ptr;

    static QString filePath(const QString &sessionId);
    static QString generateSessionId();
    static QString loadSessionId(Context *c, const QString &sessionName);
    static QString getSessionId(Context *c, const QString &sessionName);
    static QString createSessionIdIfNeeded(Context *c, quint64 expires);
    static QString createSessionId(Context *c, quint64 expires);
    static void saveSession(Context *c);
    static void deleteSession(Context *c, const QString &reason);
    static QVariant loadSession(Context *c);
    static bool validateSessionId(const QString &id);
    static quint64 extendSessionExpires(Context *c, quint64 expires);
    static quint64 getStoredSessionExpires(Context *c);

    static QVariant initializeSessionData(Context *c);
    static QDateTime saveSessionExpires(Context *c);
    static QVariant loadSessionExpires(Context *c);
    static quint64 initialSessionExpires(Context *c);
    static quint64 calculateInitialSessionExpires(Context *c);
    static quint64 resetSessionExpires(Context *c);

    SessionStore *store = 0;
    QString sessionName;
    quint64 sessionExpires;
    quint64 expiryThreshold;
    bool verifyAddress;
    bool verifyUserAgent;
};

}

#endif // SESSION_P_H
