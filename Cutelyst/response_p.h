/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYST_RESPONSE_P_H
#define CUTELYST_RESPONSE_P_H

#include "response.h"
#if (QT_VERSION < QT_VERSION_CHECK(6, 1, 0))
#    include "cookie.h"
#endif

#include <QtCore/QMap>
#include <QtCore/QUrl>
#include <QtCore/QVariant>
#include <QtNetwork/QNetworkCookie>

namespace Cutelyst {

class Context;
class Engine;
class EngineRequest;
class ResponsePrivate
{
public:
    inline ResponsePrivate(const Headers &h, EngineRequest *er)
        : headers(h)
        , engineRequest(er)
    {
    }
    inline void setBodyData(const QByteArray &body);

    Headers headers;
    QMap<QByteArray, QNetworkCookie> cookies;
#if (QT_VERSION < QT_VERSION_CHECK(6, 1, 0))
    QMap<QByteArray, Cookie> cuteCookies;
#endif
    QByteArray bodyData;
    QUrl location;
    QIODevice *bodyIODevice = nullptr;
    EngineRequest *engineRequest;
    quint16 status = Response::OK;
};

} // namespace Cutelyst

#endif // CUTELYST_RESPONSE_P_H
