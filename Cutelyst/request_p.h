/*
 * Copyright (C) 2013-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef CUTELYST_REQUEST_P_H
#define CUTELYST_REQUEST_P_H

#include "request.h"
#include "engine.h"
#include "upload.h"

#include <QtCore/QStringList>
#include <QtCore/QUrlQuery>
#include <QtCore/QUrl>
#include <QtNetwork/QHostAddress>

namespace Cutelyst {

class Engine;
class RequestPrivate
{
public:
    enum ParserStatusFlag {
        NotParsed = 0x00,
        UrlParsed = 0x01,
        BaseParsed = 0x02,
        CookiesParsed = 0x04,
        QueryParsed = 0x08,
        BodyParsed = 0x10
    };
    Q_DECLARE_FLAGS(ParserStatus, ParserStatusFlag)

    RequestPrivate(EngineRequest *er) : engineRequest(er) { }

    inline void parseUrlQuery() const;
    inline void parseBody() const;
    inline void parseCookies() const;

    static inline ParamsMultiMap parseUrlEncoded(const QByteArray &line);
    static inline QVariantMap paramsMultiMapToVariantMap(const ParamsMultiMap &params);

    // Pointer to Engine data
    Engine *engine;
    EngineRequest *engineRequest = nullptr;

    // Engines don't need to touch this
    QStringList args;
    QStringList captures;
    QString match;

    mutable QUrl url;
    mutable QString base;
    mutable QMap<QString, QString> cookies;
    mutable ParamsMultiMap queryParam;
    mutable QString queryKeywords;
    mutable ParamsMultiMap bodyParam;
    mutable QVariant bodyData;
    mutable QString remoteHostname;
    mutable QMap<QString, Upload *> uploadsMap;
    mutable QVector<Upload *> uploads;
    mutable ParserStatus parserStatus = NotParsed;
};

}

#endif // CUTELYST_REQUEST_P_H
