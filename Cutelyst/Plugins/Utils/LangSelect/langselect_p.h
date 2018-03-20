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

    void beforePrepareAction(Context *c, bool *skipMethod);
    bool detectLocale(Context *c, const QVector<LangSelect::Source> &_sourceOrder, bool *skipMethod = nullptr);

    QVector<QLocale> locales;
    QVector<LangSelect::Source> sourceOrder;
    QMap<QString,QLocale> domainMap;
    QString queryKey;
    QString sessionKey;
    QString cookieName;
    QLocale fallbackLocale;
    qint8 subDomainIdx = -127;
    qint8 pathIdx = -127;
    LangSelect::Source storeTo = LangSelect::Fallback;
    bool addContentLanguageHeader = true;
    bool autoDetect = true;
};

}

#endif // C_UTILS_LANGSELECT_P_H
