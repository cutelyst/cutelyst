/*
 * SPDX-FileCopyrightText: (C) 2018-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef C_UTILS_LANGSELECT_P_H
#define C_UTILS_LANGSELECT_P_H

#include "langselect.h"

#include <chrono>

#include <QNetworkCookie>

namespace Cutelyst {

class LangSelectPrivate
{
public:
    static void _q_postFork(Application *app);

    void beforePrepareAction(Context *c, bool *skipMethod) const;
    bool detectLocale(Context *c, LangSelect::Source _source, bool *skipMethod = nullptr) const;
    bool getFromQuery(Context *c, const QString &key) const;
    bool getFromCookie(Context *c, const QByteArray &cookie) const;
    bool getFromSession(Context *c, const QString &key) const;
    bool getFromSubdomain(Context *c, const QMap<QString, QLocale> &map) const;
    bool getFromDomain(Context *c, const QMap<QString, QLocale> &map) const;
    bool getFromHeader(Context *c,
                       const QByteArray &name = QByteArrayLiteral("Accept-Language")) const;
    void setToQuery(Context *c, const QString &key) const;
    void setToCookie(Context *c, const QByteArray &name) const;
    void setToSession(Context *c, const QString &key) const;
    void setFallback(Context *c) const;
    void setContentLanguage(Context *c) const;

    static const QString stashKeySelectionTried;

    static constexpr std::chrono::months cookieDefaultExpiration{1};

    QVector<QLocale> locales;
    LangSelect::Source source = LangSelect::Fallback;
    QMap<QString, QLocale> domainMap;
    QMap<QString, QLocale> subDomainMap;
    QStringList redirectDomains;
    QStringList redirectSubDomains;
    QString queryKey;
    QString sessionKey;
    QByteArray cookieName;
    QString cookieDomain;
    QString langStashKey = QStringLiteral("c_langselect_lang");
    QString dirStashKey  = QStringLiteral("c_langselect_dir");
    QLocale fallbackLocale;
    std::chrono::seconds cookieExpiration   = cookieDefaultExpiration;
    QNetworkCookie::SameSite cookieSameSite = QNetworkCookie::SameSite::Lax;
    bool addContentLanguageHeader           = true;
    bool autoDetect                         = true;
    bool detectFromHeader                   = true;
    bool cookieSecure                       = false;
};

} // namespace Cutelyst

#endif // C_UTILS_LANGSELECT_P_H
