/*
 * Copyright (C) 2013 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef CUTELYST_REQUEST_P_H
#define CUTELYST_REQUEST_P_H

#include "request.h"

#include "upload.h"

#include <QtCore/QStringList>
#include <QtCore/QUrlQuery>
#include <QtCore/QUrl>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QNetworkCookie>

namespace Cutelyst {

class Engine;
class RequestPrivate
{
public:
    RequestPrivate(Engine *_engine,
                   const QString &_method,
                   const QString &_path,
                   const QByteArray &_query,
                   const QString &_protocol,
                   bool _isSecure,
                   const QString &_serverAddress,
                   const QString &_remoteAddress,
                   quint16 _remotePort,
                   const QString &_remoteUser,
                   const Headers &_headers,
                   quint64 _startOfRequest,
                   QIODevice *_body,
                   void *_requestPtr);

    inline void parseBody() const;

    static inline ParamsMultiMap parseUrlEncoded(const QByteArray &line);
    static inline QVariantMap paramsMultiMapToVariantMap(const ParamsMultiMap &params);
    inline QUrl parseUrl() const;
    inline QString parseBase() const;

    // Manually filled by the Engine
    Engine *engine;
    QString method;
    // Path must not have a leading slash
    QString path;
    QByteArray query;
    QString protocol;
    QString serverAddress;
    QHostAddress remoteAddress;
    QString remoteUser;
    Headers headers;
    QIODevice *body = nullptr;
    quint64 startOfRequest;
    quint64 endOfRequest;
    // Pointer to Engine data
    void *requestPtr = nullptr;

    // Engines don't need to touch this
    QStringList args;
    QStringList captures;
    QString match;

    mutable ParamsMultiMap bodyParam;
    mutable QVariant bodyData;
    mutable QMap<QString, Upload *> uploads;

    quint16 remotePort;
    bool https = false;

    mutable bool bodyParsed = false;
};

}

#endif // CUTELYST_REQUEST_P_H
