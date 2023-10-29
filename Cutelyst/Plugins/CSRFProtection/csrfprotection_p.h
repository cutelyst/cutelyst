/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CSRFPROTECTION_P_H
#define CSRFPROTECTION_P_H

#include "csrfprotection.h"

#include <QNetworkCookie>
#include <QRegularExpression>

#include <chrono>

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

    QStringList trustedOrigins;
    static const QByteArrayList secureMethods;
    QStringList ignoredNamespaces;
    QString cookieDomain;
    QByteArray cookieName;
    QString cookiePath;
    QByteArray headerName;
    QByteArray formInputName;
    QString defaultDetachTo;
    QString errorMsgStashKey;
    QString genericErrorMessage;
    QByteArray genericContentType{"text/plain; charset=utf8"_qba};
    std::chrono::seconds cookieExpiration{0};
    static const QByteArray allowedChars;
    static const QRegularExpression sanitizeRe;
#if __cplusplus >= 202002L
    static constexpr std::chrono::years cookieDefaultExpiration{1};
#else
    static constexpr std::chrono::hours cookieDefaultExpiration{8766};
#endif
    static constexpr qsizetype secretLength{32};
    static constexpr qsizetype tokenLength{2 * secretLength};
    QNetworkCookie::SameSite cookieSameSite = QNetworkCookie::SameSite::Strict;
    bool cookieHttpOnly{false};
    bool cookieSecure{false};
    bool useSessions{false};
    bool logFailedIp{false};
};

} // namespace Cutelyst

#endif // CSRFPROTECTION_P_H
