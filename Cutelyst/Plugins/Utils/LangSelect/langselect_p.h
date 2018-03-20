/*
 * Copyright (C) 2018 Matthias Fehring <kontakt@buschmann23.de>
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

#ifndef C_UTILS_LANGSELECT_P_H
#define C_UTILS_LANGSELECT_P_H

#include "langselect.h"

namespace Cutelyst {

class LangSelectPrivate
{
public:
    static void _q_postFork(Application *app);

    void beforePrepareAction(Context *c, bool *skipMethod) const;
    bool detectLocale(Context *c, LangSelect::Source _source, bool *skipMethod = nullptr) const;
    bool getFromQuery(Context *c, const QString &key) const;
    bool getFromCookie(Context *c, const QString &cookie) const;
    bool getFromSession(Context *c, const QString &key) const;
    bool getFromPath(Context *c, int idx, bool *pathContainedValidLocale) const;
    bool getFromSubdomain(Context *c, int idx, bool *subDomainContainedValidLocale) const;
    bool getFromDomain(Context *c, const QMap<QString,QLocale> &map) const;
    bool getFromHeader(Context *c, const QString &name = QLatin1String("Accept-Language")) const;
    void setToQuery(Context *c, const QString &key) const;
    void setToCookie(Context *c, const QString &name) const;
    void setToSession(Context *c, const QString &key) const;
    void setToPath(Context *c, int idx, bool pathContainedValidLocale) const;
    void setToSubdomain(Context *c, int idx, bool subDomainContainedValidLocale) const;
    void setToDomain(Context *c, const QMap<QString,QLocale> &map) const;
    void setFallback(Context *c) const;
    void setContentLanguage(Context *c) const;

    QVector<QLocale> locales;
    LangSelect::Source source = LangSelect::Fallback;
    QMap<QString,QLocale> domainMap;
    QString queryKey;
    QString sessionKey;
    QString cookieName;
    QLocale fallbackLocale;
    qint8 subDomainIdx = -127;
    qint8 pathIdx = -127;
    bool addContentLanguageHeader = true;
    bool autoDetect = true;
    bool detectFromHeader = true;
};

}

#endif // C_UTILS_LANGSELECT_P_H
