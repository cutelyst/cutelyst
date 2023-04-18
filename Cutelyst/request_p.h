/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYST_REQUEST_P_H
#define CUTELYST_REQUEST_P_H

#include "engine.h"
#include "request.h"
#include "upload.h"

#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtNetwork/QHostAddress>

namespace Cutelyst {

class Engine;
class RequestPrivate
{
public:
    enum ParserStatusFlag {
        NotParsed     = 0x00,
        UrlParsed     = 0x01,
        BaseParsed    = 0x02,
        CookiesParsed = 0x04,
        QueryParsed   = 0x08,
        BodyParsed    = 0x10
    };
    Q_DECLARE_FLAGS(ParserStatus, ParserStatusFlag)

    inline void parseUrlQuery() const;
    inline void parseBody() const;
    inline void parseCookies() const;

    static inline QVariantMap paramsMultiMapToVariantMap(const ParamsMultiMap &params);

    // Pointer to Engine data
    Engine *engine;
    EngineRequest *engineRequest;
    QIODevice *body;

    // Engines don't need to touch this
    QStringList args;
    QStringList captures;
    QString match;

    mutable QUrl url;
    mutable QString base;
    mutable QMultiMap<QString, QString> cookies;
    mutable ParamsMultiMap queryParam;
    mutable QString queryKeywords;
    mutable ParamsMultiMap bodyParam;
    mutable QVariant bodyData;
    mutable QString remoteHostname;
    mutable QMultiMap<QString, Upload *> uploadsMap; // Cutelyst4 QStringView
    mutable QVector<Upload *> uploads;
    mutable ParserStatus parserStatus = NotParsed;
};

} // namespace Cutelyst

#endif // CUTELYST_REQUEST_P_H
