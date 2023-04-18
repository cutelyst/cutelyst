/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CSRFPROTECTION_P_H
#define CSRFPROTECTION_P_H

#include "csrfprotection.h"
#if (QT_VERSION < QT_VERSION_CHECK(6, 1, 0))
#    include "cookie.h"
#endif

#include <QNetworkCookie>
#include <QRegularExpression>

namespace Cutelyst {

class CSRFProtectionPrivate
{
public:
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

    qint64 cookieAge{0};
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
    QString genericErrorMessage;
    QString genericContentType{QStringLiteral("text/plain; charset=utf8")};
    static const QRegularExpression sanitizeRe;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 1, 0))
    QNetworkCookie::SameSite cookieSameSite = QNetworkCookie::SameSite::Strict;
#else
    Cookie::SameSite cookieSameSite = Cookie::SameSite::Strict;
#endif
    bool cookieHttpOnly{false};
    bool cookieSecure{false};
    bool useSessions{false};
    bool logFailedIp{false};
};

} // namespace Cutelyst

#endif // CSRFPROTECTION_P_H
