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

#ifndef CSRFPROTECTION_P_H
#define CSRFPROTECTION_P_H

#include "csrfprotection.h"

#include <QRegularExpression>

namespace Cutelyst {

class CSRFProtectionPrivate
{
public:
    CSRFProtectionPrivate() {}

    ~CSRFProtectionPrivate() {}

    static QByteArray getNewCsrfString();
    static QByteArray saltCipherSecret(const QByteArray &secret);
    static QByteArray unsaltCipherToken(const QByteArray &token);
    static QByteArray getNewCsrfToken();
    static QByteArray sanitizeToken(const QByteArray &token);
    static QByteArray getToken(Context *c);
    static void setToken(Context *c);
    static void reject(Context *c, const QString &logReason, const QString &displayReason);
    static void accept(Context *c);
    static bool compareSaltedTokens(const QByteArray &t1, const QByteArray &t2);

    void beforeDispatch(Context *c);

    qint64 cookieAge;
    QStringList trustedOrigins;
    static const QStringList secureMethods;
    QStringList ignoredNamespaces;
    QString cookieDomain;
    QString cookieName;
    QString cookiePath;
    QString headerName;
    QString formInputName;
    QString defaultDetachTo;
    QString errorMsgStashKey;
    static const QRegularExpression sanitizeRe;
    bool cookieHttpOnly = false;
    bool cookieSecure = false;
    bool useSessions = false;
    bool logFailedIp = false;
};

}

#endif // CSRFPROTECTION_P_H
